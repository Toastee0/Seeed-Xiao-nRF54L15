/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dmic_sample);

/* LED configuration */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define MAX_SAMPLE_RATE  16000
#define SAMPLE_BIT_WIDTH 16
#define BYTES_PER_SAMPLE sizeof(int16_t)
/* Milliseconds to wait for a block to be read. */
#define READ_TIMEOUT     1000

/* Size of a block for 100 ms of audio data. */
#define BLOCK_SIZE(_sample_rate, _number_of_channels) \
	(BYTES_PER_SAMPLE * (_sample_rate / 10) * _number_of_channels)

/* Driver will allocate blocks from this slab to receive audio data into them.
 * Application, after getting a given block from the driver and processing its
 * data, needs to free that block.
 */
#define MAX_BLOCK_SIZE   BLOCK_SIZE(MAX_SAMPLE_RATE, 2)
#define BLOCK_COUNT      4
K_MEM_SLAB_DEFINE_STATIC(mem_slab, MAX_BLOCK_SIZE, BLOCK_COUNT, 4);

/* Audio level thresholds for LED feedback */
#define THRESHOLD_LOW    500   /* Quiet sound */
#define THRESHOLD_MED    2000  /* Medium sound */
#define THRESHOLD_HIGH   8000  /* Loud sound */

/* Function to calculate RMS (Root Mean Square) amplitude */
static uint32_t calculate_rms(int16_t *buffer, size_t sample_count)
{
	uint64_t sum = 0;
	for (size_t i = 0; i < sample_count; i++) {
		int32_t val = buffer[i];
		sum += val * val;
	}
	uint32_t mean = sum / sample_count;
	/* Approximate square root */
	uint32_t rms = 0;
	for (int shift = 15; shift >= 0; shift--) {
		uint32_t test = rms | (1 << shift);
		if (test * test <= mean) {
			rms = test;
		}
	}
	return rms;
}

/* Function to draw a bar graph in the terminal */
static void draw_bar_graph(uint32_t level, uint32_t max_level)
{
	const int bar_width = 50;
	int filled = (level * bar_width) / max_level;
	if (filled > bar_width) {
		filled = bar_width;
	}
	
	printk("[");
	for (int i = 0; i < bar_width; i++) {
		if (i < filled) {
			printk("=");
		} else {
			printk(" ");
		}
	}
	printk("] %5u\n", level);
}

static int do_pdm_transfer(const struct device *dmic_dev,
			   struct dmic_cfg *cfg,
			   size_t block_count)
{
	int ret;

	LOG_INF("PCM output rate: %u, channels: %u",
		cfg->streams[0].pcm_rate, cfg->channel.req_num_chan);

	ret = dmic_configure(dmic_dev, cfg);
	if (ret < 0) {
		LOG_ERR("Failed to configure the driver: %d", ret);
		return ret;
	}

	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (ret < 0) {
		LOG_ERR("START trigger failed: %d", ret);
		return ret;
	}

	printk("\n=== Audio Level Monitor (press Ctrl+C to stop) ===\n");
	printk("LED States: OFF=quiet, SLOW=medium, FAST=loud\n\n");

	for (int i = 0; i < block_count; ++i) {
		void *buffer;
		uint32_t size;

		ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
		if (ret < 0) {
			printk("\n%d - read failed: %d\n", i, ret);
			return ret;
		}

		/* Calculate audio level (RMS amplitude) */
		size_t sample_count = size / BYTES_PER_SAMPLE;
		uint32_t rms_level = calculate_rms((int16_t *)buffer, sample_count);

		/* Draw bar graph (only update display every other block to reduce overhead) */
		if ((i % 2) == 0) {
			draw_bar_graph(rms_level, 10000);  /* Max scale set to 10k */
		}

		/* Control LED based on audio level */
		if (rms_level < THRESHOLD_LOW) {
			/* Quiet - LED off */
			gpio_pin_set_dt(&led, 0);
		} else if (rms_level < THRESHOLD_MED) {
			/* Medium - LED blinks slowly (toggle every 500ms) */
			if ((i % 5) == 0) {
				gpio_pin_toggle_dt(&led);
			}
		} else if (rms_level < THRESHOLD_HIGH) {
			/* Loud - LED blinks fast (toggle every 200ms) */
			if ((i % 2) == 0) {
				gpio_pin_toggle_dt(&led);
			}
		} else {
			/* Very loud - LED stays on */
			gpio_pin_set_dt(&led, 1);
		}

		k_mem_slab_free(&mem_slab, buffer);
	}

	printk("\n");

	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
	if (ret < 0) {
		LOG_ERR("STOP trigger failed: %d", ret);
		return ret;
	}

	return ret;
}

int main(void)
{
	const struct device *const dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));
	int ret;

	LOG_INF("DMIC sample with LED and visual feedback");

	if (!device_is_ready(dmic_dev)) {
		LOG_ERR("%s is not ready", dmic_dev->name);
		return 0;
	}

	/* Initialize LED */
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED device not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED: %d", ret);
		return 0;
	}

	struct pcm_stream_cfg stream = {
		.pcm_width = SAMPLE_BIT_WIDTH,
		.mem_slab  = &mem_slab,
	};
	struct dmic_cfg cfg = {
		.io = {
			/* These fields can be used to limit the PDM clock
			 * configurations that the driver is allowed to use
			 * to those supported by the microphone.
			 */
			.min_pdm_clk_freq = 1000000,
			.max_pdm_clk_freq = 3500000,
			.min_pdm_clk_dc   = 40,
			.max_pdm_clk_dc   = 60,
		},
		.streams = &stream,
		.channel = {
			.req_num_streams = 1,
		},
	};

	/* Start with single channel (mono) for continuous monitoring */
	cfg.channel.req_num_chan = 1;
	cfg.channel.req_chan_map_lo =
		dmic_build_channel_map(0, 0, PDM_CHAN_LEFT);
	cfg.streams[0].pcm_rate = MAX_SAMPLE_RATE;
	cfg.streams[0].block_size =
		BLOCK_SIZE(cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);

	/* Run continuously - use a large block count for extended monitoring */
	LOG_INF("Starting continuous audio monitoring...");
	ret = do_pdm_transfer(dmic_dev, &cfg, 1000); /* Monitor for ~100 seconds */
	if (ret < 0) {
		gpio_pin_set_dt(&led, 0); /* Turn off LED on error */
		return 0;
	}

	/* Turn off LED when done */
	gpio_pin_set_dt(&led, 0);
	LOG_INF("Exiting");
	return 0;
}