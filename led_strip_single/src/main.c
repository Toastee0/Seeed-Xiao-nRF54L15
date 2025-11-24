/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#define STRIP_NODE		DT_ALIAS(led_strip)

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define DELAY_TIME K_MSEC(CONFIG_SAMPLE_LED_UPDATE_DELAY)

/* Maximum brightness for smooth, vibrant animations */
#define MAX_BRIGHTNESS CONFIG_SAMPLE_LED_BRIGHTNESS
#define GLOW_BRIGHTNESS (MAX_BRIGHTNESS / 4)  /* 25% brightness for gentle glow */

/* Color definitions at 25% brightness */
#define COLOR_RED    {.r = GLOW_BRIGHTNESS, .g = 0, .b = 0}
#define COLOR_BLUE   {.r = 0, .g = 0, .b = GLOW_BRIGHTNESS}
#define COLOR_GREEN  {.r = 0, .g = GLOW_BRIGHTNESS, .b = 0}

static struct led_rgb pixel;
static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

/* Helper function for smooth linear interpolation between colors */
static void lerp_color(const struct led_rgb *from, const struct led_rgb *to, float t, struct led_rgb *result)
{
	result->r = (uint8_t)(from->r + (to->r - from->r) * t);
	result->g = (uint8_t)(from->g + (to->g - from->g) * t);
	result->b = (uint8_t)(from->b + (to->b - from->b) * t);
}

/* Smooth glowing transition between red, blue, and green */
static void animation_smooth_glow(uint32_t step)
{
	/* Define our three colors at 25% brightness */
	static const struct led_rgb red = COLOR_RED;
	static const struct led_rgb blue = COLOR_BLUE;
	static const struct led_rgb green = COLOR_GREEN;
	
	/* Each color transition takes 500ms 
	 * At 20ms per step, that's 25 steps per transition (500/20 = 25)
	 * Full cycle: red->blue (25) + blue->green (25) + green->red (25) = 75 steps
	 */
	const uint32_t steps_per_transition = 25;  /* 500ms / 20ms = 25 steps */
	const uint32_t cycle_length = steps_per_transition * 3;  /* 3 transitions */
	
	uint32_t position = step % cycle_length;
	float transition_progress;
	
	if (position < steps_per_transition) {
		/* Red to Blue transition */
		transition_progress = (float)position / steps_per_transition;
		lerp_color(&red, &blue, transition_progress, &pixel);
	} else if (position < steps_per_transition * 2) {
		/* Blue to Green transition */
		transition_progress = (float)(position - steps_per_transition) / steps_per_transition;
		lerp_color(&blue, &green, transition_progress, &pixel);
	} else {
		/* Green to Red transition */
		transition_progress = (float)(position - steps_per_transition * 2) / steps_per_transition;
		lerp_color(&green, &red, transition_progress, &pixel);
	}
}

int main(void)
{
	int rc;
	uint32_t step = 0;

	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

	LOG_INF("Starting smooth RGB LED glow");
	LOG_INF("Glowing at 25%% brightness (%d/255)", GLOW_BRIGHTNESS);
	LOG_INF("Cycle: Red -> Blue -> Green -> Red (500ms each transition)");

	while (1) {
		/* Run smooth glow animation */
		animation_smooth_glow(step);

		/* Update the LED */
		rc = led_strip_update_rgb(strip, &pixel, 1);
		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		step++;
		k_sleep(DELAY_TIME);
	}

	return 0;
}
