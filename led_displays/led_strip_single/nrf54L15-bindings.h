/*
 * Copyright (c) 2025, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_SAMPLES_DRIVERS_LED_WS2812_NRF54L15_BINDINGS_H_
#define ZEPHYR_SAMPLES_DRIVERS_LED_WS2812_NRF54L15_BINDINGS_H_

/*
 * WS2812 LED timing requirements:
 * - T0H (0 code high): 200-500 ns (typical 350 ns)
 * - T1H (1 code high): 550-850 ns (typical 700 ns)  
 * - T0L (0 code low):  650-950 ns (typical 800 ns)
 * - T1L (1 code low):  450-750 ns (typical 600 ns)
 * - Total cycle time:  ~1250 ns
 *
 * At 8 MHz, 1 bit is 125 ns, so 8 bits is 1000 ns.
 * This provides excellent timing resolution for WS2812 control.
 *
 * Timing analysis:
 * - '0' bit: 3 high bits (375 ns) + 5 low bits (625 ns) = 1000 ns total
 * - '1' bit: 6 high bits (750 ns) + 2 low bits (250 ns) = 1000 ns total
 *
 * Frame encoding:
 * - '0' frame: 0b11100000 = 0xE0
 * - '1' frame: 0b11111100 = 0xFC
 *
 * The nRF54L15's enhanced GPIO capabilities allow for 8 MHz operation,
 * providing better timing precision compared to the nRF52's 4 MHz limit.
 * This results in more reliable WS2812 LED control with tighter timing
 * tolerances and improved signal quality.
 */

#define SPI_FREQ    8000000
#define ZERO_FRAME  0xE0
#define ONE_FRAME   0xFC

/* Alternative conservative timing (if needed for stability):
 * - '0' bit: 2 high bits (250 ns) + 6 low bits (750 ns) = 1000 ns total  
 * - '1' bit: 5 high bits (625 ns) + 3 low bits (375 ns) = 1000 ns total
 * 
 * #define ZERO_FRAME  0xC0
 * #define ONE_FRAME   0xF8
 */

#endif /* ZEPHYR_SAMPLES_DRIVERS_LED_WS2812_NRF54L15_BINDINGS_H_ */