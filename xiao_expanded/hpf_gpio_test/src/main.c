/*
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Blinky using HPF GPIO on FLPR core
 * Based on Zephyr blinky sample
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hpf_gpio_blinky, LOG_LEVEL_INF);

/* Use the onboard LED (LED0) */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;
	bool led_state = true;

	LOG_INF("===========================================");
	LOG_INF("HPF GPIO Blinky Test");
	LOG_INF("Testing GPIO via FLPR core");
	LOG_INF("===========================================");

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO device not ready");
		return -1;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED pin: %d", ret);
		return ret;
	}

	LOG_INF("LED configured successfully on %s pin %d", 
			led.port->name, led.pin);
	LOG_INF("Starting blink loop...");

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			LOG_ERR("Failed to toggle LED: %d", ret);
			return ret;
		}

		led_state = !led_state;
		LOG_INF("LED: %s", led_state ? "ON" : "OFF");
		
		k_msleep(1000);
	}

	return 0;
}
