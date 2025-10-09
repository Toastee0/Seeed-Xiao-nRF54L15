#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(main_app, CONFIG_LOG_DEFAULT_LEVEL);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios); // Get the expansion base button device from the device tree alias
static const struct gpio_dt_spec xiao_button = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios); // Get the XIAO button device from the device tree alias

static const struct gpio_dt_spec relay = GPIO_DT_SPEC_GET(DT_ALIAS(relay0), gpios); // Get the relay device from the device tree alias

int main(void)
{
    int ret;

    LOG_INF("Starting Zephyr button and relay example...");

    /* Check if GPIO devices are ready */
    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("Button device %s is not ready", button.port->name); 
        return -1;
    }

    if (!gpio_is_ready_dt(&xiao_button)) {
        LOG_ERR("XIAO button device %s is not ready", xiao_button.port->name); 
        return -1;
    }

    if (!gpio_is_ready_dt(&relay)) {
        LOG_ERR("Relay device %s is not ready", relay.port->name);
        return -1;
    }

    /* Configure button pins as input mode */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Failed to configure %s pin %d (error %d)", button.port->name, button.pin, ret);
        return -1;
    }

    ret = gpio_pin_configure_dt(&xiao_button, GPIO_INPUT | GPIO_PULL_UP);
    if (ret != 0) {
        LOG_ERR("Failed to configure XIAO %s pin %d (error %d)", xiao_button.port->name, xiao_button.pin, ret);
        return -1;
    }

    /* Configure relay pin as output mode */
    ret = gpio_pin_configure_dt(&relay, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Failed to configure %s pin %d (error %d)", relay.port->name, relay.pin, ret);
        return -1;
    }

    LOG_INF("Press the built-in XIAO button (sw0) to toggle the relay...");
    LOG_INF("Hold the external button on D1 (sw1) for momentary relay control...");

    bool relay_state = false;
    bool button_pressed_prev = false;

    while (1) {
        /* Read both button states */
        int button_state_raw = gpio_pin_get_dt(&button);
        int xiao_button_state_raw = gpio_pin_get_dt(&xiao_button);

        /* Check if reads are successful */
        if (button_state_raw < 0) {
            LOG_ERR("Error reading expansion button pin: %d", button_state_raw);
            return -1;
        }
        if (xiao_button_state_raw < 0) {
            LOG_ERR("Error reading XIAO button pin: %d", xiao_button_state_raw);
            return -1;
        }

        bool button_pressed = (button_state_raw == 0); // Xiao Built-in button pressed (ACTIVE_LOW)
        bool xiao_button_pressed = (xiao_button_state_raw == 0); // Expansion board Button(D1) pressed (ACTIVE_HIGH)
        
        /* Debug logging for button states - remove this later */
        static int debug_counter = 0;
        if (debug_counter++ % 100 == 0) { // Log every 100 cycles to avoid spam
            LOG_INF("Button states - sw0 (built-in): %d, sw1 (D1): %d", button_state_raw, xiao_button_state_raw);
        }

        /* Handle built-in XIAO button - toggle functionality */
        if (button_pressed && !button_pressed_prev) {
            relay_state = !relay_state; // Toggle relay state
            
            /* Only set relay state if external D1 button is not being pressed */
            if (!xiao_button_pressed) {
                gpio_pin_set_dt(&relay, relay_state);
            }
            
            /* Since relay is GPIO_ACTIVE_LOW, the actual relay state is inverted */
            if (relay_state) {
                LOG_INF("relay off (built-in XIAO button toggle)");
            } else {
                LOG_INF("relay on (built-in XIAO button toggle)");
            }
        }

        /* Handle XIAO button - momentary functionality (overrides toggle state) */
        static bool xiao_button_pressed_prev = false;
        static bool xiao_momentary_logged = false;
        
        if (xiao_button_pressed && !xiao_button_pressed_prev) {
            gpio_pin_set_dt(&relay, false); // Turn relay on (remember: active low)
            LOG_INF("relay on (D1 button momentary)");
            xiao_momentary_logged = true;
        } else if (!xiao_button_pressed && xiao_button_pressed_prev && xiao_momentary_logged) {
            /* XIAO button released, restore toggle state */
            gpio_pin_set_dt(&relay, relay_state);
            LOG_INF("relay restored to toggle state");
            xiao_momentary_logged = false;
        } else if (xiao_button_pressed) {
            /* Keep relay on while button is held, but don't spam logs */
            gpio_pin_set_dt(&relay, false);
        }
        
        xiao_button_pressed_prev = xiao_button_pressed;

        button_pressed_prev = button_pressed;
        k_msleep(50); /* Debounce delay */
    }
    return 0;
}