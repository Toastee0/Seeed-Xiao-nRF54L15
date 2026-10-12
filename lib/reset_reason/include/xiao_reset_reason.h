/*
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef XIAO_RESET_REASON_H_
#define XIAO_RESET_REASON_H_

#include <stdint.h>

/**
 * @brief Raw RESETREAS value captured at boot.
 *
 * Returns 0 after a power-on reset or brownout (the hardware clears the
 * register), or when CONFIG_XIAO_RESET_REASON is disabled.
 */
#ifdef CONFIG_XIAO_RESET_REASON
uint32_t xiao_reset_reason_get(void);
#else
static inline uint32_t xiao_reset_reason_get(void)
{
	return 0;
}
#endif

#endif /* XIAO_RESET_REASON_H_ */
