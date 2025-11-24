/*
 * HPF GPIO Blinky Test with RTT output
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hpf_test, LOG_LEVEL_INF);

#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;
	int counter = 0;

	LOG_INF("HPF GPIO Test Starting...");
	
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO device not ready!");
		return -1;
	}

	LOG_INF("GPIO device ready, configuring LED...");

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED GPIO: %d", ret);
		return -1;
	}

	LOG_INF("LED configured successfully, starting blink loop...");

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			LOG_ERR("Failed to toggle LED: %d", ret);
			return -1;
		}
		
		LOG_INF("LED toggle %d (HPF GPIO working)", ++counter);
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}