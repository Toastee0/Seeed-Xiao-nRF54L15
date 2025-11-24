/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uni_hid_device, CONFIG_BLUEPAD32_LOG_LEVEL);

/* Stub: HID device management */

int uni_hid_device_init(void)
{
	LOG_DBG("HID device module initialized");
	return 0;
}
