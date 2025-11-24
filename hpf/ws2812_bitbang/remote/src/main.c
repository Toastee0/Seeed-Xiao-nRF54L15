/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 * 
 * WS2812 LED strip test on FLPR core
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>

#define STRIP_NODE DT_ALIAS(led_strip)
#define STRIP_NUM_LEDS DT_PROP(STRIP_NODE, chain_length)

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);

int main(void)
{
	struct led_rgb pixels[STRIP_NUM_LEDS];
	uint16_t hue = 0;
	int ret;

	if (!device_is_ready(strip)) {
		return -1;
	}

	while (1) {
		/* Rainbow effect - cycle through hues */
		for (int i = 0; i < STRIP_NUM_LEDS; i++) {
			uint16_t led_hue = (hue + (i * 65536 / STRIP_NUM_LEDS)) % 65536;
			
			/* Simple HSV to RGB conversion */
			uint8_t r, g, b;
			if (led_hue < 21845) {  /* Red to Yellow */
				r = 32; g = (led_hue * 32) / 21845; b = 0;
			} else if (led_hue < 43690) {  /* Yellow to Green */
				r = 32 - ((led_hue - 21845) * 32) / 21845; g = 32; b = 0;
			} else {  /* Green to Blue to Red */
				r = 0; g = 32; b = ((led_hue - 43690) * 32) / 21846;
			}
			
			pixels[i].r = r;
			pixels[i].g = g;
			pixels[i].b = b;
		}

		ret = led_strip_update_rgb(strip, pixels, STRIP_NUM_LEDS);
		if (ret) {
			/* Error - could add error handling */
		}

		hue += 256;  /* Slow rainbow rotation */
		k_msleep(50);
	}

	return 0;
}
