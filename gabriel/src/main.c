
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <nrfx_power.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel_version.h>
#include <ncs_version.h>
#include <zephyr/sys/util.h>
#include <zephyr/data/json.h>

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/regulator.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define BUF_SIZE 256

const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart21));

static const char *TRIGGER = "getID";
static const char *TRIGGER1 = "getVERSION";

static uint8_t get_hw_id[32];
static ssize_t get_hw_id_len = 0;

static char rx_buf[BUF_SIZE];
static int rx_pos = 0;

static volatile bool send_hw_pending;
static volatile bool send_ver_pending;

static char uart_tx_buf[BUF_SIZE];
static volatile int uart_tx_pos;
static volatile int uart_tx_len;

static void uart_send(const char *data, size_t len)
{
    if (uart_tx_len > 0) {
        return; /* previous TX still in progress, drop */
    }
    size_t copy = MIN(len, sizeof(uart_tx_buf));
    memcpy(uart_tx_buf, data, copy);
    uart_tx_pos = 0;
    uart_tx_len = copy;
    uart_irq_tx_enable(uart_dev);
}

// Json struct
struct hw_msg
{
    const char *HW_ID;
};

struct version_msg
{
    const char *app_build;
    const char *ncs_build;
    const char *kernel;
};
// in Json formatieren
static const struct json_obj_descr hw_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct hw_msg, HW_ID, JSON_TOK_STRING)};

static const struct json_obj_descr version_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct version_msg, app_build, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct version_msg, ncs_build, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct version_msg, kernel, JSON_TOK_STRING),
};
// Batterie setting
#define VBAT_REG_NODE DT_NODELABEL(vbat_pwr)
#define ADC_NODE DT_NODELABEL(adc)
#define VBAT_ADC_CH_NODE DT_CHILD(ADC_NODE, channel_7)

static const struct device *const vbat_reg = DEVICE_DT_GET(VBAT_REG_NODE);
static const struct device *const adc_dev = DEVICE_DT_GET(ADC_NODE);

static const struct adc_channel_cfg vbat_chan_cfg = ADC_CHANNEL_CFG_DT(VBAT_ADC_CH_NODE);

static int read_battery_mv(int32_t *out_mv)
{
    int err;
    uint16_t raw = 0;

    if (!device_is_ready(adc_dev))
    {
        return -ENODEV;
    }
    if (!device_is_ready(vbat_reg))
    {
        return -ENODEV;
    }

    /* Enable the battery sense switch (VBAT_EN via vbat_pwr) */
    err = regulator_enable(vbat_reg);
    if (err < 0)
    {
        return err;
    }
    k_sleep(K_MSEC(5));

    /* Configure ADC channel (as defined in board DTS) */
    err = adc_channel_setup(adc_dev, &vbat_chan_cfg);
    if (err < 0)
    {
        regulator_disable(vbat_reg);
        return err;
    }

    struct adc_sequence seq = {
        .channels = BIT(vbat_chan_cfg.channel_id),
        .buffer = &raw,
        .buffer_size = sizeof(raw),
        .resolution = DT_PROP(VBAT_ADC_CH_NODE, zephyr_resolution),
        .oversampling = DT_PROP_OR(VBAT_ADC_CH_NODE, zephyr_oversampling, 0),
    };

    err = adc_read(adc_dev, &seq);
    regulator_disable(vbat_reg);

    if (err < 0)
    {
        return err;
    }

    /* RAW -> mV at the ADC pin */
    int32_t mv = (int32_t)raw;
    uint16_t ref_mv = adc_ref_internal(adc_dev);
    err = adc_raw_to_millivolts((int32_t)ref_mv, vbat_chan_cfg.gain, (uint8_t)seq.resolution, &mv);
    if (err < 0)
    {
        return err;
    }

    /* Board has 10k/10k divider => ADC sees VBAT/2, so scale back */
    mv *= 2;

    *out_mv = mv;
    return 0;
}

void send_hardware(const struct device *dev)
{
    struct hw_msg msg;
    char tx_buf[256];
    char hw_id_hex[2 * sizeof(get_hw_id)];
    int ret;
    int n = 0;

    if (get_hw_id_len <= 0)
    {
        msg.HW_ID = "unavailable";
        // n = snprintf(tx_buf, sizeof(tx_buf), "HW-ID: <unavailable>\n");
    }
    else
    {

        // Hardware ID Länge + ganze Zahl in Hexa
        // n += snprintf(tx_buf + n, sizeof(tx_buf) - n, "%02X", get_hw_id[0]);

        for (int i = 1; i < get_hw_id_len && n < (int)sizeof(hw_id_hex); i++)
        {
            n += snprintf(&hw_id_hex[n], sizeof(hw_id_hex) - n, "%02X", get_hw_id[i]);
        }

        hw_id_hex[n] = '\0';
        msg.HW_ID = hw_id_hex;

        // zeilenende + neue Zeile
        // n += snprintf(tx_buf + n, sizeof(tx_buf) - n, "\n");
    }
    ret = json_obj_encode_buf(hw_msg_descr,
                              ARRAY_SIZE(hw_msg_descr),
                              &msg,
                              tx_buf,
                              sizeof(tx_buf));

    if (ret < 0)
    {
        snprintf(tx_buf, sizeof(tx_buf),
                 "{\"hw_id\":\"error\"}");
    }

    size_t len = strnlen(tx_buf, sizeof(tx_buf));
    if (len < sizeof(tx_buf) - 1)
    {
        tx_buf[len++] = '\n';
        tx_buf[len] = '\0';
    }

    uart_send(tx_buf, len);
}

void send_version(const struct device *dev)
{
    struct version_msg msg;
    char tx_buf[256];
    char ncs_buf[256];
    char app_buf[256];
    char kernel_buf[256];

    int ret;

    // ncs_build_version
    const char *ncs_build = STRINGIFY(NCS_BUILD_VERSION);
    snprintf(ncs_buf, sizeof(ncs_buf), "NCS_BUILD_VERSION: %s", ncs_build);

    // app_build_version
    const char *app_build = STRINGIFY(APP_BUILD_VERSION);
    snprintf(app_buf, sizeof(app_buf), "APP_BUILD_VERSION: %s",
             app_build);

    // sys_kernel_version
    uint32_t v = sys_kernel_version_get();
    snprintf(kernel_buf, sizeof(kernel_buf),
             "SYS_KERNEL_VERSION: %u.%u.%u",
             SYS_KERNEL_VER_MAJOR(v),
             SYS_KERNEL_VER_MINOR(v),
             SYS_KERNEL_VER_PATCHLEVEL(v));

    msg.ncs_build = ncs_buf;
    msg.app_build = app_buf;
    msg.kernel = kernel_buf;

    ret = json_obj_encode_buf(version_msg_descr,
                              ARRAY_SIZE(version_msg_descr),
                              &msg,
                              tx_buf,
                              sizeof(tx_buf));

    if (ret < 0)
    {
        snprintf(tx_buf, sizeof(tx_buf),
                 "{\"error\":\"version_encode_failed\"}");
    }

    size_t len = strnlen(tx_buf, sizeof(tx_buf));
    if (len < sizeof(tx_buf) - 1)
    {
        tx_buf[len++] = '\n';
        tx_buf[len] = '\0';
    }

    uart_send(tx_buf, len);
}

// ISR-Uart-Interrupt Serivce Routine
static void uart_isr(const struct device *dev, void *user_data)
{

    if (!uart_irq_update(dev))
    {
        return;
    }

    /* TX: feed bytes from uart_tx_buf into FIFO */
    if (uart_irq_tx_ready(dev))
    {
        if (uart_tx_pos < uart_tx_len) {
            int sent = uart_fifo_fill(dev,
                (uint8_t *)&uart_tx_buf[uart_tx_pos],
                uart_tx_len - uart_tx_pos);
            uart_tx_pos += sent;
        }
        if (uart_tx_pos >= uart_tx_len) {
            uart_irq_tx_disable(dev);
            uart_tx_len = 0;
        }
    }

    if (!uart_irq_rx_ready(dev))
    {
        return;
    }

    uint8_t c;
    int rec;

    // Alle aktuell verfügbaren Bytes abholen
    while ((rec = uart_fifo_read(dev, &c, 1)) > 0)
    {

        // Zeichenende erkennen
        if (c == '\n') //|| c == '\r')
        {
            // Null-terminieren
            if (rx_pos < (int)sizeof(rx_buf))
            {
                rx_buf[rx_pos] = '\0';
            }
            else
            {
                rx_buf[sizeof(rx_buf) - 1] = '\0';
            }

            // Vergleich mit Text - nur Flag setzen, main loop erledigt den Rest
            if (strcmp(rx_buf, TRIGGER) == 0)
            {
                send_hw_pending = true;
            }
            if (strcmp(rx_buf, TRIGGER1) == 0)
            {
                send_ver_pending = true;
            }

            // Für nächste Zeile zurücksetzen
            rx_pos = 0;
            continue;
        }

        // Normales Zeichen puffern Überlaufschutz
        if (rx_pos < (int)sizeof(rx_buf) - 1)
        {
            rx_buf[rx_pos++] = c;
        }
        else
        {
            // Zu lange Zeile: verwerfen und neu beginnen
            rx_pos = 0;
        }
    }
}

int main(void)
{
    /* Port 1 ist ein always on bzw. low-power-freunldiche Domäne: stabile versorgung und
       weckt das System zuverlässig
       Port 2 ist ein High-Performance Domäne bei schnellen Signalen wie UART RX kann dann
       das Startbit verpasst werden, außer man hält die Domäne wach mit
       nrfx_power_constlat_mode_request(), passt das Power-Managment an mit konstanter Latenz

       Port 2.08 (TX) und 2.07 (RX) von UART21 werden mit verschiedenen spannung
       Betrieben um diese aufzuheben muss die nrfx_power_constlat_mode_request()
       Eingefügt werden anderenfalls kann man im xiao_nrf54l15_nrfl15_cpuapp.overlay
       die Pins anderes konfigurieren zum Beispiel P1.04 und P1.05 dann würde Uart21
       funktionieren.
   */
    nrfx_power_constlat_mode_request();

    if (!device_is_ready(uart_dev))
    {
        printk("Uart Device not ready\n");
        return -1;
    }

    // Hardware-ID holen
    get_hw_id_len = hwinfo_get_device_id(get_hw_id, sizeof(get_hw_id));

    // Uart interrupt einstellen
    uart_irq_callback_user_data_set(uart_dev, uart_isr, NULL);

    uart_irq_rx_enable(uart_dev);

    int ret;
    bool led_is_on = true;

    if (!gpio_is_ready_dt(&led))
    {
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        return ret;
    }

    while (1)
    {

        ret = gpio_pin_set_dt(&led, (int)led_is_on);
		 if (ret < 0) {
			 return ret;
		 }
		 led_is_on = !led_is_on;



        int32_t vbat_mv;
        int err = read_battery_mv(&vbat_mv);

        if (err == 0)
        {
            printk("VBAT = %d mV (%d.%03d V)\n",
                   (int)vbat_mv,
                   (int)(vbat_mv / 1000),
                   (int)(vbat_mv % 1000));
        }
        else
        {
            printk("VBAT read failed: %d\n", err);
        }

        if (send_hw_pending) {
            send_hw_pending = false;
            send_hardware(uart_dev);
        }
        if (send_ver_pending) {
            send_ver_pending = false;
            send_version(uart_dev);
        }

        k_sleep(K_SECONDS(1));
        printk("main enable\n");

        // Consolen ausgabe
        printk("sys_kernel_version_get: %d\n", sys_kernel_version_get());
        printk("APP_BUILD_VERSION: %s\n", STRINGIFY(APP_BUILD_VERSION));
        printk("NCS_BUILD_VERSION: %s\n", STRINGIFY(NCS_BUILD_VERSION));
    }

    return 0;
}
