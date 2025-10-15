/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

LOG_MODULE_REGISTER(BLE_Basic_Xiao, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)

// Xiao board hardware for led & button
//static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Get BT info set in conf file
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_LED_BLINK_INTERVAL 1000

// Advertising packet
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

// URL data for scan response
static unsigned char url_data[] = {0x17, '/', '/', 'a', 'c', 'a', 'd', 'e', 'm', 'y', '.',
                                   'n', 'o', 'r', 'd', 'i', 'c', 's', 'e', 'm', 'i', '.',
                                   'c', 'o', 'm'};

// Scan response packet
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_URI, url_data, sizeof(url_data)),
};

static bool led_is_on = false;

// Function to toggle the LED on the Xiao board
static int xiao_toggle_led(const struct gpio_dt_spec *led)
{
	int ret;

	if (!led || !gpio_is_ready_dt(led)) {
		return -ENODEV;
	}

	led_is_on = !led_is_on;
	ret = gpio_pin_set_dt(led, led_is_on ? 1 : 0);
	if (ret < 0) {
		LOG_ERR("Failed to set LED state: %d", ret);
		return ret;
	}

	return 0;
}

int main(void)
{
	int err;
	
	LOG_INF("Starting Basic BLE Sample for Xiao nRF54L15\n");

	// Configure LED GPIO
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED device %s is not ready", led.port->name);
		return -1;
	}

	err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		LOG_ERR("Failed to configure LED (err %d)\n", err);
		return err;
	}

	// Enable the Bluetooth LE stack
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");

	// Start advertising
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}

	LOG_INF("Advertising successfully started\n");

	for (;;) {
		xiao_toggle_led(&led);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
