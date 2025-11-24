#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
#include <math.h>

LOG_MODULE_REGISTER(pwm_rgb_example, CONFIG_LOG_DEFAULT_LEVEL);

// PWM period: 1ms (1kHz frequency)
#define PWM_PERIOD_NS 1000000UL

// Get RGB LED PWM specs from device tree
#define RED_PWM_LED   DT_ALIAS(red_pwm_led)
#define GREEN_PWM_LED DT_ALIAS(green_pwm_led)
#define BLUE_PWM_LED  DT_ALIAS(blue_pwm_led)

static const struct pwm_dt_spec red_led = PWM_DT_SPEC_GET(RED_PWM_LED);
static const struct pwm_dt_spec green_led = PWM_DT_SPEC_GET(GREEN_PWM_LED);
static const struct pwm_dt_spec blue_led = PWM_DT_SPEC_GET(BLUE_PWM_LED);

// Button for wake-up
#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;

// Wake-up flag
static volatile bool wakeup_requested = false;

/**
 * @brief Button interrupt handler for wake-up
 */
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    wakeup_requested = true;
    LOG_INF("Button wake-up triggered");
}

/**
 * @brief Set RGB LED color with 0-255 values
 */
static int set_rgb_color(uint8_t r, uint8_t g, uint8_t b)
{
    int ret;
    uint32_t red_duty, green_duty, blue_duty;

    // Convert 0-255 to duty cycle in nanoseconds
    red_duty = (PWM_PERIOD_NS * r) / 255U;
    green_duty = (PWM_PERIOD_NS * g) / 255U;
    blue_duty = (PWM_PERIOD_NS * b) / 255U;

    ret = pwm_set_dt(&red_led, PWM_PERIOD_NS, red_duty);
    if (ret < 0) return ret;

    ret = pwm_set_dt(&green_led, PWM_PERIOD_NS, green_duty);
    if (ret < 0) return ret;

    ret = pwm_set_dt(&blue_led, PWM_PERIOD_NS, blue_duty);
    return ret;
}

/**
 * @brief Convert HSV to RGB
 * H: 0-360, S: 0-100, V: 0-100
 */
static void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float c = v * s / 100.0f;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v / 100.0f - c;
    float r_prime, g_prime, b_prime;

    if (h < 60) {
        r_prime = c; g_prime = x; b_prime = 0;
    } else if (h < 120) {
        r_prime = x; g_prime = c; b_prime = 0;
    } else if (h < 180) {
        r_prime = 0; g_prime = c; b_prime = x;
    } else if (h < 240) {
        r_prime = 0; g_prime = x; b_prime = c;
    } else if (h < 300) {
        r_prime = x; g_prime = 0; b_prime = c;
    } else {
        r_prime = c; g_prime = 0; b_prime = x;
    }

    *r = (uint8_t)((r_prime + m) * 255.0f);
    *g = (uint8_t)((g_prime + m) * 255.0f);
    *b = (uint8_t)((b_prime + m) * 255.0f);
}

/**
 * @brief Smoothly fade from one color to another
 */
static void fade_to_color(uint8_t from_r, uint8_t from_g, uint8_t from_b,
                         uint8_t to_r, uint8_t to_g, uint8_t to_b,
                         int steps, int delay_ms)
{
    for (int i = 0; i <= steps; i++) {
        uint8_t r = from_r + ((to_r - from_r) * i) / steps;
        uint8_t g = from_g + ((to_g - from_g) * i) / steps;
        uint8_t b = from_b + ((to_b - from_b) * i) / steps;
        set_rgb_color(r, g, b);
        k_msleep(delay_ms);
    }
}

int main(void)
{
    int ret;
    uint8_t r, g, b;
    float hue = 0.0f;

    LOG_INF("Starting RGB LED PWM example with button wake...");

    // Check if all PWM devices are ready
    if (!device_is_ready(red_led.dev) || 
        !device_is_ready(green_led.dev) || 
        !device_is_ready(blue_led.dev)) {
        LOG_ERR("One or more PWM devices not ready");
        return -ENODEV;
    }

    // Configure button for wake-up
    if (!device_is_ready(button.port)) {
        LOG_ERR("Button device not ready");
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure button GPIO: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure button interrupt: %d", ret);
        return ret;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    LOG_INF("Button wake-up configured on sw0");
    LOG_INF("All PWM channels ready. Period: %lu ns (1kHz)", PWM_PERIOD_NS);
    LOG_INF("RGB LED on D1 (Red), D2 (Green), D3 (Blue)");
    LOG_INF("Press button to wake from battery power");

    wakeup_requested = true;  // Start active

    while (1) {
        // Rainbow effect - smooth color cycle through full spectrum
        LOG_INF("Rainbow effect...");
        for (int i = 0; i < 360; i += 2) {
            hue = (float)i;
            hsv_to_rgb(hue, 100.0f, 100.0f, &r, &g, &b);
            
            ret = set_rgb_color(r, g, b);
            if (ret < 0) {
                LOG_ERR("Failed to set RGB color: %d", ret);
                return ret;
            }
            
            k_msleep(20);
        }

        // Fade to white
        LOG_INF("Fading to white...");
        hsv_to_rgb(0, 100.0f, 100.0f, &r, &g, &b);  // Start from red
        fade_to_color(r, g, b, 255, 255, 255, 50, 20);
        k_msleep(1000);

        // Fade to black
        LOG_INF("Fading to black...");
        fade_to_color(255, 255, 255, 0, 0, 0, 50, 20);
        k_msleep(500);

        // Smooth primary colors sequence with fades
        LOG_INF("Primary colors with smooth transitions...");
        fade_to_color(0, 0, 0, 255, 0, 0, 50, 10);      // Fade to Red
        k_msleep(500);
        fade_to_color(255, 0, 0, 0, 255, 0, 50, 10);    // Red to Green
        k_msleep(500);
        fade_to_color(0, 255, 0, 0, 0, 255, 50, 10);    // Green to Blue
        k_msleep(500);
        
        // Smooth secondary colors with fades
        LOG_INF("Secondary colors with smooth transitions...");
        fade_to_color(0, 0, 255, 255, 255, 0, 50, 10);  // Blue to Yellow
        k_msleep(500);
        fade_to_color(255, 255, 0, 0, 255, 255, 50, 10); // Yellow to Cyan
        k_msleep(500);
        fade_to_color(0, 255, 255, 255, 0, 255, 50, 10); // Cyan to Magenta
        k_msleep(500);
        fade_to_color(255, 0, 255, 0, 0, 0, 50, 10);     // Magenta to Black
        k_msleep(500);
    }

    return 0;
}