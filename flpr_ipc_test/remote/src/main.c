/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flpr_remote, LOG_LEVEL_INF);

#define IPC_DATA_SIZE 32

/* IPC message structure */
struct ipc_data_packet {
	uint32_t counter;
	uint32_t timestamp;
	char message[IPC_DATA_SIZE];
};

static struct ipc_ept ep;
static volatile bool ep_bound = false;

/* Callback when endpoint is bound */
static void ep_bound_cb(void *priv)
{
	ep_bound = true;
	LOG_INF("FLPR: IPC endpoint bound");
}

/* Callback when data is received from CPU app */
static void ep_recv_cb(const void *data, size_t len, void *priv)
{
	struct ipc_data_packet recv_packet;
	struct ipc_data_packet send_packet;
	int ret;

	if (len != sizeof(struct ipc_data_packet)) {
		LOG_ERR("FLPR: Invalid data size received: %d", len);
		return;
	}

	/* Copy received data */
	memcpy(&recv_packet, data, sizeof(recv_packet));

	LOG_INF("FLPR: Received counter=%u, timestamp=%u, msg='%s'",
		recv_packet.counter, recv_packet.timestamp, recv_packet.message);

	/* Process the data: double the counter, add 1000 to timestamp */
	send_packet.counter = recv_packet.counter * 2;
	send_packet.timestamp = recv_packet.timestamp + 1000;
	snprintf(send_packet.message, IPC_DATA_SIZE, "FLPR_ACK_%u", recv_packet.counter);

	/* Send modified data back to CPU app */
	ret = ipc_service_send(&ep, &send_packet, sizeof(send_packet));
	if (ret < 0) {
		LOG_ERR("FLPR: Failed to send data back (err %d)", ret);
	} else {
		LOG_INF("FLPR: Sent back counter=%u, timestamp=%u, msg='%s'",
			send_packet.counter, send_packet.timestamp, send_packet.message);
	}
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
	int ret;

	LOG_INF("FLPR core IPC test application started");

	/* Get IPC instance */
	ipc_instance = DEVICE_DT_GET(DT_NODELABEL(ipc0));
	if (!device_is_ready(ipc_instance)) {
		LOG_ERR("IPC instance not ready");
		return -ENODEV;
	}

	/* Open IPC service */
	ret = ipc_service_open_instance(ipc_instance);
	if (ret < 0 && ret != -EALREADY) {
		LOG_ERR("Failed to open IPC instance (err %d)", ret);
		return ret;
	}

	/* Register endpoint */
	ret = ipc_service_register_endpoint(ipc_instance, &ep, &ep_cfg);
	if (ret < 0) {
		LOG_ERR("Failed to register endpoint (err %d)", ret);
		return ret;
	}

	/* Wait for endpoint to be bound */
	LOG_INF("FLPR: Waiting for endpoint to bind...");
	while (!ep_bound) {
		k_msleep(1);
	}

	LOG_INF("FLPR: Ready and waiting for data from CPU app");

	/* Main loop - just sleep, IPC callbacks handle everything */
	while (1) {
		k_msleep(1000);
	}

	return 0;
}
