/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 * 
 * WS2812 LED Strip Driver for nRF54L15 FLPR (RISC-V) Core
 * Ported from zephyr/drivers/led_strip/ws2812_gpio.c
 */

#define DT_DRV_COMPAT worldsemi_ws2812_flpr

#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/dt-bindings/led/led.h>
#include <hal/nrf_vpr_csr.h>
#include <hal/nrf_vpr_csr_vio.h>

#define LOG_LEVEL CONFIG_LED_STRIP_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ws2812_flpr);

/* 
 * WS2812 Timing Requirements (nanoseconds):
 * T0H (0 bit high): 200-500 ns (target: 350 ns)
 * T1H (1 bit high): 550-850 ns (target: 700 ns)
 * T0L (0 bit low):  650-950 ns (target: 800 ns)
 * T1L (1 bit low):  450-750 ns (target: 600 ns)
 * 
 * FLPR Core: 128 MHz RISC-V (7.8125 ns per cycle)
 * Timing in cycles:
 * T0H: 350ns / 7.8125ns = ~45 cycles
 * T1H: 700ns / 7.8125ns = ~90 cycles
 * T0L: 800ns / 7.8125ns = ~102 cycles
 * T1L: 600ns / 7.8125ns = ~77 cycles
 */

#define FLPR_FREQ_MHZ 128
#define NS_TO_CYCLES(ns) (((ns) * FLPR_FREQ_MHZ) / 1000)

#define CYCLES_T0H NS_TO_CYCLES(350)  /* ~45 cycles */
#define CYCLES_T1H NS_TO_CYCLES(700)  /* ~90 cycles */
#define CYCLES_T0L NS_TO_CYCLES(800)  /* ~102 cycles */
#define CYCLES_T1L NS_TO_CYCLES(600)  /* ~77 cycles */

struct ws2812_flpr_cfg {
	uint8_t vio_pin;        /* VIO pin number (0-15) */
	uint8_t num_colors;     /* Number of color channels (3 for RGB, 4 for RGBW) */
	const uint8_t *color_mapping;
	size_t length;          /* Number of LEDs in strip */
};

/* Inline cycle-accurate delay for RISC-V */
static inline void delay_cycles(uint32_t cycles)
{
	/* Simple loop: each iteration ~4 cycles (load, compare, branch, decrement) */
	cycles = cycles / 4;
	while (cycles--) {
		__asm__ volatile("nop");
	}
}

/* Set VIO pin high */
static inline void vio_pin_set(uint8_t pin)
{
	nrf_vpr_csr_vio_out_set(1U << pin);
}

/* Set VIO pin low */
static inline void vio_pin_clear(uint8_t pin)
{
	uint32_t current = nrf_vpr_csr_vio_out_get();
	nrf_vpr_csr_vio_out_set(current & ~(1U << pin));
}

/* Send a single bit to WS2812 */
static inline void send_bit(uint8_t pin, bool bit_value)
{
	if (bit_value) {
		/* Send '1' bit: long high pulse, short low pulse */
		vio_pin_set(pin);
		delay_cycles(CYCLES_T1H);
		vio_pin_clear(pin);
		delay_cycles(CYCLES_T1L);
	} else {
		/* Send '0' bit: short high pulse, long low pulse */
		vio_pin_set(pin);
		delay_cycles(CYCLES_T0H);
		vio_pin_clear(pin);
		delay_cycles(CYCLES_T0L);
	}
}

/* Send buffer to WS2812 LEDs */
static int send_buf(const struct device *dev, uint8_t *buf, size_t len)
{
	const struct ws2812_flpr_cfg *config = dev->config;
	uint8_t pin = config->vio_pin;
	unsigned int key;

	/* Disable interrupts for precise timing */
	key = irq_lock();

	/* Send each byte MSB first */
	while (len--) {
		uint8_t byte = *buf++;
		
		/* Send 8 bits, MSB first */
		for (int i = 7; i >= 0; i--) {
			send_bit(pin, byte & (1 << i));
		}
	}

	irq_unlock(key);

	/* WS2812 latch time: >50us low - already satisfied by function return */
	return 0;
}

/* Update RGB pixels - LED strip API function */
static int ws2812_flpr_update_rgb(const struct device *dev,
                                   struct led_rgb *pixels,
                                   size_t num_pixels)
{
	const struct ws2812_flpr_cfg *config = dev->config;
	uint8_t *ptr = (uint8_t *)pixels;
	size_t i;

	/* Convert from RGB to on-wire format (e.g. GRB, GRBW, RGB, etc) */
	for (i = 0; i < num_pixels; i++) {
		uint8_t j;

		for (j = 0; j < config->num_colors; j++) {
			switch (config->color_mapping[j]) {
			case LED_COLOR_ID_WHITE:
				*ptr++ = 0;  /* White channel not supported */
				break;
			case LED_COLOR_ID_RED:
				*ptr++ = pixels[i].r;
				break;
			case LED_COLOR_ID_GREEN:
				*ptr++ = pixels[i].g;
				break;
			case LED_COLOR_ID_BLUE:
				*ptr++ = pixels[i].b;
				break;
			default:
				return -EINVAL;
			}
		}
	}

	return send_buf(dev, (uint8_t *)pixels, num_pixels * config->num_colors);
}

/* Get LED strip length - LED strip API function */
static size_t ws2812_flpr_length(const struct device *dev)
{
	const struct ws2812_flpr_cfg *config = dev->config;
	return config->length;
}

/* LED strip API */
static DEVICE_API(led_strip, ws2812_flpr_api) = {
	.update_rgb = ws2812_flpr_update_rgb,
	.length = ws2812_flpr_length,
};

/* Device initialization */
#define WS2812_FLPR_INIT(idx)                                           \
                                                                        \
static int ws2812_flpr_##idx##_init(const struct device *dev)           \
{                                                                       \
	const struct ws2812_flpr_cfg *cfg = dev->config;                \
	uint8_t i;                                                      \
                                                                        \
	/* Validate color mapping */                                    \
	for (i = 0; i < cfg->num_colors; i++) {                         \
		switch (cfg->color_mapping[i]) {                        \
		case LED_COLOR_ID_WHITE:                                \
		case LED_COLOR_ID_RED:                                  \
		case LED_COLOR_ID_GREEN:                                \
		case LED_COLOR_ID_BLUE:                                 \
			break;                                          \
		default:                                                \
			LOG_ERR("%s: invalid color mapping", dev->name);\
			return -EINVAL;                                 \
		}                                                       \
	}                                                               \
                                                                        \
	/* Configure VIO pin as output */                               \
	nrf_vpr_csr_vio_dir_set(1U << cfg->vio_pin);                    \
	vio_pin_clear(cfg->vio_pin);                                    \
                                                                        \
	LOG_INF("WS2812 FLPR initialized on VIO pin %d", cfg->vio_pin); \
	return 0;                                                       \
}                                                                       \
                                                                        \
static const uint8_t ws2812_flpr_##idx##_color_mapping[] =              \
	DT_INST_PROP(idx, color_mapping);                               \
                                                                        \
static const struct ws2812_flpr_cfg ws2812_flpr_##idx##_cfg = {         \
	.vio_pin = DT_INST_PROP(idx, vio_pin),                          \
	.num_colors = DT_INST_PROP_LEN(idx, color_mapping),             \
	.color_mapping = ws2812_flpr_##idx##_color_mapping,             \
	.length = DT_INST_PROP(idx, chain_length),                      \
};                                                                      \
                                                                        \
DEVICE_DT_INST_DEFINE(idx,                                              \
		    ws2812_flpr_##idx##_init,                           \
		    NULL,                                               \
		    NULL,                                               \
		    &ws2812_flpr_##idx##_cfg,                           \
		    POST_KERNEL,                                        \
		    CONFIG_LED_STRIP_INIT_PRIORITY,                     \
		    &ws2812_flpr_api);

DT_INST_FOREACH_STATUS_OKAY(WS2812_FLPR_INIT)
