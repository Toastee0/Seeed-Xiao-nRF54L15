/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

LOG_MODULE_REGISTER(bt_connection, CONFIG_BLUEPAD32_LOG_LEVEL);

/* Stub: Connection management functions will be implemented here */

int bt_connection_init(void)
{
	LOG_DBG("Connection module initialized");
	return 0;
}
