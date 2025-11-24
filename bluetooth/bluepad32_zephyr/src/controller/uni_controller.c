/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uni_controller, CONFIG_BLUEPAD32_LOG_LEVEL);

/* Stub: Unified controller abstraction */

int uni_controller_init(void)
{
	LOG_DBG("Controller module initialized");
	return 0;
}
