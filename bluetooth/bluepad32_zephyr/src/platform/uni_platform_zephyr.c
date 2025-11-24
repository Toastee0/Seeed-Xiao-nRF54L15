/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uni_platform_zephyr, CONFIG_BLUEPAD32_LOG_LEVEL);

/* Stub: Zephyr platform layer */

int uni_platform_zephyr_init(void)
{
	LOG_INF("Bluepad32 Zephyr platform initialized");
	return 0;
}
