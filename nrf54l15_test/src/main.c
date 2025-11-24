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
#include <app_version.h>
#include <ncs_version.h>
#include <zephyr/sys/util.h>
#include <zephyr/data/json.h>

#define BUF_SIZE 256

const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart21));

static const char *TRIGGER = "getID";
static const char *TRIGGER1 = "getVERSION";

static uint8_t get_hw_id[32];
static ssize_t get_hw_id_len = 0;

static char rx_buf[BUF_SIZE];
static int rx_pos = 0;

//Json struct 
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
//in Json formatieren
static const struct json_obj_descr hw_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct hw_msg, HW_ID, JSON_TOK_STRING)};

static const struct json_obj_descr version_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct version_msg, app_build, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct version_msg, ncs_build, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct version_msg, kernel, JSON_TOK_STRING),
};

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

    // Tatsächlich vorhandene Nutzlänge sicher bestimmen
    // size_t len = strnlen(tx_buf, sizeof(tx_buf));

    // Interrupt Deaktivieren
    // uart_irq_rx_disable(dev);
    // printk("Rx-disabled\n");

    // Senden über Polling Uart
    for (size_t i = 0; i < len; i++)
    {
        uart_poll_out(dev, (uint8_t)tx_buf[i]);
    }

    // Interrupt Aktivieren
    // uart_irq_rx_enable(dev);
    // printk("Rx-enable\n");
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
    snprintf(app_buf, sizeof(app_buf), "APP_BUILD_VERSION: v%s-%s",
             APP_VERSION_STRING, app_build);

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

    for (size_t i = 0; i < len; i++)
    {
        uart_poll_out(dev, (uint8_t)tx_buf[i]);
    }
}

// ISR-Uart-Interrupt Serivce Routine
static void uart_isr(const struct device *dev, void *user_data)
{

    if (!uart_irq_update(dev))
    {
        return;
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

            // Vergleich mit Text
            if (strcmp(rx_buf, TRIGGER) == 0)
            {
                send_hardware(dev);
            }
            if (strcmp(rx_buf, TRIGGER1) == 0)
            {
                send_version(dev);
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

    while (1)
    {

        k_sleep(K_SECONDS(1));
        printk("main enable\n");

        // Consolen ausgabe
        printk("sys_kernel_version_get: %d\n", sys_kernel_version_get());
        printk("APP_BUILD_VERSION: v%s-%s\n", APP_VERSION_STRING, STRINGIFY(APP_BUILD_VERSION));
        printk("NCS_BUILD_VERSION: %s\n", STRINGIFY(NCS_BUILD_VERSION));
    }

    return 0;
}
