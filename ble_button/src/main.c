/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * Adapted for XIAO nRF54L15 by Toastee0
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

LOG_MODULE_REGISTER(BLE_Button_Xiao, LOG_LEVEL_INF);

/* GPIO definitions for XIAO nRF54L15 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_cb_data;

/* Company identifier (Company ID) */
#define COMPANY_ID_CODE 0x0059

/* Structure for custom advertising data */
typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code */
	uint16_t number_press; /* Number of times button is pressed */
} adv_mfg_data_type;

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_LED_BLINK_INTERVAL 1000

/* LE Advertising Parameters */
static const struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE, /* No options specified */
			800, /* Min Advertising Interval 500ms (800*0.625ms) */
			801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
			NULL); /* Set to NULL for undirected advertising */

/* Initialize manufacturer data with company code and zero press count */
static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE, 0x00 };

static unsigned char url_data[] = { 0x17, '/', '/', 'a', 'c', 'a', 'd', 'e', 'm',
				    'y',  '.', 'n', 'o', 'r', 'd', 'i', 'c', 's',
				    'e',  'm', 'i', '.', 'c', 'o', 'm' };

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	/* Include the Manufacturer Specific Data in the advertising packet */
	BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_URI, url_data, sizeof(url_data)),
};

/* Button interrupt callback function - updates advertising data dynamically */
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	bool button_state = !gpio_pin_get_dt(&button0); /* Active low button */
	
	if (button_state) { /* Button pressed */
		adv_mfg_data.number_press += 1;
		
		LOG_INF("Button pressed! Count: %d", adv_mfg_data.number_press);
		
		/* Update advertising data with new press count */
		int err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
		if (err) {
			LOG_ERR("Failed to update advertising data (err %d)", err);
		}
		
		/* Flash LED to indicate button press */
		gpio_pin_set_dt(&led0, 1);
		k_msleep(100);
		gpio_pin_set_dt(&led0, 0);
	}
}

/* Initialize button with GPIO interrupt */
static int init_button(void)
{
	int err;

	if (!gpio_is_ready_dt(&button0)) {
		LOG_ERR("Error: button device %s is not ready", button0.port->name);
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	if (err != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d", err, button0.port->name, button0.pin);
		return err;
	}

	err = gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_BOTH);
	if (err != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", err, button0.port->name, button0.pin);
		return err;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button0.pin));
	gpio_add_callback(button0.port, &button_cb_data);
	
	LOG_INF("Set up button at %s pin %d", button0.port->name, button0.pin);

	return 0;
}

int main(void)
{
	int blink_status = 0;
	int err;

	LOG_INF("Starting BLE Button Sample for XIAO nRF54L15");

	/* Initialize LED */
	if (!gpio_is_ready_dt(&led0)) {
		LOG_ERR("Error: LED device %s is not ready", led0.port->name);
		return -1;
	}

	err = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	if (err < 0) {
		LOG_ERR("Error %d: failed to configure LED device %s pin %d", err, led0.port->name, led0.pin);
		return -1;
	}

	/* Setup button */
	err = init_button();
	if (err) {
		LOG_ERR("Button init failed (err %d)", err);
		return -1;
	}

	/* Enable Bluetooth */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return -1;
	}

	LOG_INF("Bluetooth initialized");

	/* Start advertising */
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return -1;
	}

	LOG_INF("Advertising successfully started");
	LOG_INF("Press button to increment counter in advertising data");

	for (;;) {
		/* Blink LED to show running status */
		gpio_pin_set_dt(&led0, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}