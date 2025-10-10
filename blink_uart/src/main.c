#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/poweroff.h>


#define SLEEP_TIME_MS   1000
#define LOOP_COUNT      6

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;
	bool led_state = true;
	int count = 0;
	k_msleep(SLEEP_TIME_MS);
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	while (count < LOOP_COUNT) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
		printk("%s\n", led_state ? "Yes the real Slim Shady!\n" : "I'm Slim Shady\n");
		k_msleep(SLEEP_TIME_MS);
		count++;
	}
	
	// Turn off LED before entering system off mode
	gpio_pin_set_dt(&led, 1);
	
	printk("Entering system off mode...\n");
	k_msleep(100);  // Give time for print to complete
	sys_poweroff();
	
	return 0;
}