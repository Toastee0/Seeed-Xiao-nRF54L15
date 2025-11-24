/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(cpuapp_main, LOG_LEVEL_INF);

#define IPC_DATA_SIZE 32
#define LED0_NODE DT_ALIAS(led0)

/* IPC message structure - must match FLPR side */
struct ipc_data_packet {
	uint32_t counter;
	uint32_t timestamp;
	char message[IPC_DATA_SIZE];
};

static struct ipc_ept ep;
static volatile bool ep_bound = false;
static volatile bool response_received = false;
static struct ipc_data_packet last_response;

/* LED for visual feedback */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0});

/* Callback when endpoint is bound */
static void ep_bound_cb(void *priv)
{
	ep_bound = true;
	LOG_INF("CPU APP: IPC endpoint bound");
}

/* Callback when data is received from FLPR */
static void ep_recv_cb(const void *data, size_t len, void *priv)
{
	if (len != sizeof(struct ipc_data_packet)) {
		LOG_ERR("CPU APP: Invalid data size received: %d", len);
		return;
	}

	/* Copy received data */
	memcpy(&last_response, data, sizeof(last_response));
	response_received = true;

	LOG_INF("CPU APP: Response from FLPR - counter=%u, timestamp=%u, msg='%s'",
		last_response.counter, last_response.timestamp, last_response.message);
}

static struct ipc_ept_cfg ep_cfg = {
	.name = "flpr_ep",
	.cb = {
		.bound = ep_bound_cb,
		.received = ep_recv_cb,
	},
};

int main(void)
{
	const struct device *ipc_instance;
	struct ipc_data_packet send_packet;
	uint32_t counter = 0;
	int ret;
	bool led_available = false;

	LOG_INF("CPU APP: FLPR IPC Test Application");
	LOG_INF("==========================================");

	/* Initialize LED if available */
	if (led.port && device_is_ready(led.port)) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
		if (ret == 0) {
			led_available = true;
			LOG_INF("CPU APP: LED initialized");
		}
	}

	/* Get IPC instance */
	ipc_instance = DEVICE_DT_GET(DT_NODELABEL(ipc0));
	if (!device_is_ready(ipc_instance)) {
		LOG_ERR("CPU APP: IPC instance not ready");
		return -ENODEV;
	}

	/* Open IPC service */
	ret = ipc_service_open_instance(ipc_instance);
	if (ret < 0 && ret != -EALREADY) {
		LOG_ERR("CPU APP: Failed to open IPC instance (err %d)", ret);
		return ret;
	}

	/* Register endpoint */
	ret = ipc_service_register_endpoint(ipc_instance, &ep, &ep_cfg);
	if (ret < 0) {
		LOG_ERR("CPU APP: Failed to register endpoint (err %d)", ret);
		return ret;
	}

	/* Wait for endpoint to be bound */
	LOG_INF("CPU APP: Waiting for endpoint to bind...");
	while (!ep_bound) {
		k_msleep(10);
	}

	LOG_INF("CPU APP: IPC ready! Starting data exchange...");
	LOG_INF("==========================================");

	/* Main loop - send data periodically */
	while (1) {
		/* Prepare data packet */
		send_packet.counter = counter;
		send_packet.timestamp = k_uptime_get_32();
		snprintf(send_packet.message, IPC_DATA_SIZE, "APP_MSG_%u", counter);

		LOG_INF("CPU APP: Sending counter=%u, timestamp=%u, msg='%s'",
			send_packet.counter, send_packet.timestamp, send_packet.message);

		/* Send to FLPR */
		response_received = false;
		ret = ipc_service_send(&ep, &send_packet, sizeof(send_packet));
		if (ret < 0) {
			LOG_ERR("CPU APP: Failed to send data (err %d)", ret);
		} else {
			/* Wait for response with timeout */
			int timeout_count = 0;
			while (!response_received && timeout_count < 100) {
				k_msleep(10);
				timeout_count++;
			}

			if (response_received) {
				LOG_INF("CPU APP: ✓ Round-trip successful!");
				LOG_INF("CPU APP:   Sent:     counter=%u", send_packet.counter);
				LOG_INF("CPU APP:   Received: counter=%u (doubled by FLPR)", 
					last_response.counter);
				LOG_INF("CPU APP:   Timestamp delta: %u ms", 
					last_response.timestamp - send_packet.timestamp);
				
				/* Toggle LED on successful communication */
				if (led_available) {
					gpio_pin_toggle_dt(&led);
				}
			} else {
				LOG_WRN("CPU APP: ✗ No response from FLPR (timeout)");
			}
		}

		counter++;
		LOG_INF("------------------------------------------");
		
		/* Wait 2 seconds between transmissions */
		k_msleep(2000);
	}

	return 0;
}
