/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>

LOG_MODULE_REGISTER(bt_scan, CONFIG_BLUEPAD32_LOG_LEVEL);

/* Stub: Scanning functions will be implemented here */

int bt_scan_init(void)
{
	LOG_DBG("Scan module initialized");
	return 0;
}
