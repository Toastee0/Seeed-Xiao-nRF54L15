/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <zephyr/kernel.h>
 #include <zephyr/device.h>
 #include <zephyr/devicetree.h>
 #include <zephyr/drivers/gpio.h>
 #include <zephyr/sys/printk.h>
 #include <nrfx_power.h>
 #include <zephyr/drivers/uart.h>

 /* 1000 msec = 1 sec */
 #define SLEEP_TIME_MS   1000

 
/* STEP 10.1.1 - Define the size of the receive buffer */
#define RECEIVE_BUFF_SIZE 10

/* STEP 10.2 - Define the receiving timeout period */
#define RECEIVE_TIMEOUT 100

#if defined(CONFIG_BOARD_XIAO_NRF54L15_NRF54L15_CPUAPP) || defined(CONFIG_BOARD_XIAO_NRF54L15_NRF54L15_CPUAPP_NS)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
#endif

#if defined(CONFIG_BOARD_XIAO_NRF54L15_NRF54L15_CPUAPP) 
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart20));
#else
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
#endif

 
 /* The devicetree node identifier for the "led0" alias. */
 #define LED0_NODE DT_ALIAS(led0)
 
 /*
  * Get the GPIO specification for the LED
  */
 static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
 const struct device *uart= DEVICE_DT_GET(DT_NODELABEL(uart20));
 
 int main(void)
 {
	 int ret;
	 bool led_is_on = true;
	nrfx_power_constlat_mode_request();
	 if (!gpio_is_ready_dt(&led)) {
		 return -1;
	 }
 
	 ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	 if (ret < 0) {
		 return ret;
	 }
 
	 while (1) {
		 ret = gpio_pin_set_dt(&led, (int)led_is_on);
		 if (ret < 0) {
			 return ret;
		 }
		 led_is_on = !led_is_on;
		 k_msleep(SLEEP_TIME_MS);
	 }
 
	 return 0;
 }