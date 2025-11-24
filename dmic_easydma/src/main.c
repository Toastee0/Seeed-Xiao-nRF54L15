/*
 * DMIC EasyDMA Sample
 * 
 * Demonstrates efficient DMIC audio capture using DMA buffers
 * optimized for feeding LC3 codec in BLE Audio applications.
 * 
 * Key features:
 * - Continuous DMIC capture at 16kHz (LC3 compatible)
 * - Double-buffered DMA approach for zero-copy operation
 * - Real-time RMS monitoring
 * - Ring buffer for consumer thread (simulating LC3 encoder)
 */

#include <zephyr/kernel.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/device.h>
#include <zephyr/sys/ring_buffer.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dmic_easydma, LOG_LEVEL_INF);

/* Audio configuration matching BLE Audio LC3 requirements */
#define SAMPLE_RATE      16000  /* 16kHz for LC3 */
#define SAMPLE_BIT_WIDTH 16     /* 16-bit samples */
#define BYTES_PER_SAMPLE 2
#define CHANNELS         1      /* Mono */

/* 10ms blocks - LC3 frame duration */
#define FRAME_DURATION_MS 10
#define BLOCK_SIZE_SAMPLES (SAMPLE_RATE * FRAME_DURATION_MS / 1000)  /* 160 samples */
#define BLOCK_SIZE_BYTES (BLOCK_SIZE_SAMPLES * BYTES_PER_SAMPLE)     /* 320 bytes */

/* DMA buffer pool - 4 blocks for continuous operation */
#define BLOCK_COUNT 4
K_MEM_SLAB_DEFINE_STATIC(dma_mem_slab, BLOCK_SIZE_BYTES, BLOCK_COUNT, 4);

/* Ring buffer for producer-consumer pattern */
#define RING_BUF_FRAMES 8  /* Hold 8 frames (80ms) */
#define RING_BUF_SIZE (BLOCK_SIZE_BYTES * RING_BUF_FRAMES)
RING_BUF_DECLARE(audio_ring_buf, RING_BUF_SIZE);

/* Statistics */
static uint32_t blocks_captured = 0;
static uint32_t blocks_consumed = 0;
static uint32_t ring_buffer_overflows = 0;

/* Global DMIC device */
static const struct device *g_dmic_dev = NULL;

/* Calculate RMS level efficiently */
static uint32_t calculate_rms(int16_t *buffer, size_t sample_count)
{
	uint64_t sum = 0;
	for (size_t i = 0; i < sample_count; i++) {
		int32_t val = buffer[i];
		sum += val * val;
	}
	uint32_t mean = sum / sample_count;
	
	/* Fast integer square root */
	uint32_t rms = 0;
	for (int shift = 15; shift >= 0; shift--) {
		uint32_t test = rms | (1 << shift);
		if (test * test <= mean) {
			rms = test;
		}
	}
	return rms;
}

/* DMIC capture thread - producer */
static void dmic_capture_thread(void *arg1, void *arg2, void *arg3)
{
	void *buffer;
	uint32_t size;
	int ret;

	/* Wait for device to be set */
	while (g_dmic_dev == NULL) {
		k_sleep(K_MSEC(10));
	}

	LOG_INF("DMIC capture thread started");

	while (true) {
		/* Read from DMIC DMA buffer */
		ret = dmic_read(g_dmic_dev, 0, &buffer, &size, SYS_FOREVER_MS);
		if (ret < 0) {
			LOG_ERR("DMIC read failed: %d", ret);
			k_sleep(K_MSEC(10));
			continue;
		}

		blocks_captured++;

		/* Try to put into ring buffer for consumer */
		uint32_t space = ring_buf_space_get(&audio_ring_buf);
		if (space >= size) {
			uint32_t written = ring_buf_put(&audio_ring_buf, buffer, size);
			if (written != size) {
				LOG_WRN("Ring buffer partial write: %u/%u", written, size);
			}
		} else {
			ring_buffer_overflows++;
			if ((ring_buffer_overflows % 100) == 1) {
				LOG_WRN("Ring buffer overflow! Captured: %u, Consumed: %u, Overflows: %u",
					blocks_captured, blocks_consumed, ring_buffer_overflows);
			}
		}

		/* Free the DMA buffer back to pool */
		k_mem_slab_free(&dma_mem_slab, buffer);

		/* Show stats every 100 blocks (~1 second) */
		if ((blocks_captured % 100) == 0) {
			uint32_t used = ring_buf_size_get(&audio_ring_buf);
			LOG_INF("Captured: %u blocks, Ring buf: %u/%u bytes (%.1f%% full)",
				blocks_captured, used, RING_BUF_SIZE, 
				(used * 100.0f) / RING_BUF_SIZE);
		}
	}
}

/* Audio consumer thread - simulates LC3 encoder */
static void audio_consumer_thread(void *arg1, void *arg2, void *arg3)
{
	int16_t frame_buffer[BLOCK_SIZE_SAMPLES];
	uint32_t block_count = 0;

	LOG_INF("Audio consumer thread started (simulating LC3 encoder)");

	/* Wait a bit for capture to start filling buffer */
	k_sleep(K_MSEC(100));

	while (true) {
		/* Read one frame from ring buffer */
		uint32_t bytes_read = ring_buf_get(&audio_ring_buf, (uint8_t *)frame_buffer, 
						   BLOCK_SIZE_BYTES);
		
		if (bytes_read == BLOCK_SIZE_BYTES) {
			blocks_consumed++;
			block_count++;

			/* Calculate RMS every 50 blocks (~500ms) */
			if ((block_count % 50) == 0) {
				uint32_t rms = calculate_rms(frame_buffer, BLOCK_SIZE_SAMPLES);
				
				/* Debug: Check if samples are all the same (bad) or varying (good) */
				int16_t min_val = frame_buffer[0];
				int16_t max_val = frame_buffer[0];
				for (int i = 1; i < BLOCK_SIZE_SAMPLES; i++) {
					if (frame_buffer[i] < min_val) min_val = frame_buffer[i];
					if (frame_buffer[i] > max_val) max_val = frame_buffer[i];
				}
				
				LOG_INF("Audio RMS: %5u | Range: %d to %d (span=%d)", 
					rms, min_val, max_val, max_val - min_val);
			}

			/* Simulate LC3 encoding time (~5ms for 10ms frame) */
			k_sleep(K_MSEC(5));

		} else if (bytes_read > 0) {
			LOG_WRN("Partial frame read: %u bytes", bytes_read);
		} else {
			/* Ring buffer empty - wait for more data */
			k_sleep(K_MSEC(5));
		}
	}
}

K_THREAD_DEFINE(capture_tid, 2048, dmic_capture_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(consumer_tid, 2048, audio_consumer_thread, NULL, NULL, NULL, 8, 0, 0);

int main(void)
{
	const struct device *dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));
	int ret;

	LOG_INF("=== DMIC EasyDMA Sample ===");
	LOG_INF("Sample Rate: %d Hz", SAMPLE_RATE);
	LOG_INF("Frame Size: %d samples (%d bytes, %d ms)", 
		BLOCK_SIZE_SAMPLES, BLOCK_SIZE_BYTES, FRAME_DURATION_MS);
	LOG_INF("DMA Blocks: %d", BLOCK_COUNT);
	LOG_INF("Ring Buffer: %d bytes (%d frames)", RING_BUF_SIZE, RING_BUF_FRAMES);

	if (!device_is_ready(dmic_dev)) {
		LOG_ERR("DMIC device not ready!");
		return -ENODEV;
	}

	/* Configure DMIC */
	struct pcm_stream_cfg stream = {
		.pcm_width = SAMPLE_BIT_WIDTH,
		.mem_slab = &dma_mem_slab,
	};

	struct dmic_cfg cfg = {
		.io = {
			.min_pdm_clk_freq = 1000000,
			.max_pdm_clk_freq = 3500000,
			.min_pdm_clk_dc = 40,
			.max_pdm_clk_dc = 60,
		},
		.streams = &stream,
		.channel = {
			.req_num_streams = 1,
			.req_num_chan = CHANNELS,
			.req_chan_map_lo = dmic_build_channel_map(0, 0, PDM_CHAN_LEFT),
		},
	};

	cfg.streams[0].pcm_rate = SAMPLE_RATE;
	cfg.streams[0].block_size = BLOCK_SIZE_BYTES;

	LOG_INF("Configuring DMIC...");
	ret = dmic_configure(dmic_dev, &cfg);
	if (ret < 0) {
		LOG_ERR("Failed to configure DMIC: %d", ret);
		return ret;
	}

	LOG_INF("Starting DMIC DMA capture...");
	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (ret < 0) {
		LOG_ERR("Failed to start DMIC: %d", ret);
		return ret;
	}

	LOG_INF("DMIC running! Threads will process audio continuously.");
	LOG_INF("Speak into microphone to see RMS levels change.\n");

	/* Set global device pointer */
	g_dmic_dev = dmic_dev;

	/* Threads will start automatically via K_THREAD_DEFINE */

	/* Main thread just monitors status */
	while (true) {
		k_sleep(K_SECONDS(10));
		LOG_INF("=== Status ===");
		LOG_INF("Captured: %u blocks, Consumed: %u blocks", 
			blocks_captured, blocks_consumed);
		LOG_INF("Ring buffer overflows: %u", ring_buffer_overflows);
		LOG_INF("Performance: %.1f%% (100%% = perfect match)",
			(blocks_consumed * 100.0f) / blocks_captured);
	}

	return 0;
}
