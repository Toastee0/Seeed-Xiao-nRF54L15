/*
 * Audio Visualizer Module for LED Matrix
 * Adapted from xiao_expanded/audio_visualizer
 */

#include "audio_viz.h"
#include <string.h>
#include <math.h>
#include <zephyr/kernel.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/logging/log.h>
#include <arm_math.h>

LOG_MODULE_REGISTER(audio_viz, LOG_LEVEL_INF);

// Audio buffer configuration
#define SAMPLE_BIT_WIDTH 16
#define BYTES_PER_SAMPLE sizeof(int16_t)
#define BLOCK_SIZE(_sample_rate, _number_of_channels) \
	(BYTES_PER_SAMPLE * (_sample_rate / 10) * _number_of_channels)
#define MAX_BLOCK_SIZE   BLOCK_SIZE(AUDIO_SAMPLE_RATE, 1)
#define BLOCK_COUNT      4
#define READ_TIMEOUT     1000

K_MEM_SLAB_DEFINE_STATIC(audio_mem_slab, MAX_BLOCK_SIZE, BLOCK_COUNT, 4);

// FFT buffers
static float32_t fft_input[AUDIO_FFT_SIZE * 2];
static float32_t fft_output[AUDIO_FFT_SIZE];
static arm_rfft_fast_instance_f32 fft_instance;

// Mel filterbank
static float mel_filterbank[AUDIO_MEL_BANDS][AUDIO_FFT_SIZE / 2];
static uint8_t mel_values[AUDIO_MEL_BANDS];
static uint8_t mel_values_smoothed[AUDIO_MEL_BANDS];

// Beat detection
static uint8_t bass_history[8];
static uint8_t bass_history_idx = 0;
static bool beat_detected = false;

// DMIC device
static const struct device *dmic_dev;
static bool audio_available = false;
static bool audio_running = false;

// Smoothing factors
#define SMOOTHING_FACTOR 0.2f  // 0.0 = no smoothing, 1.0 = full smoothing

// Convert frequency to mel scale
static float hz_to_mel(float hz) {
    return 2595.0f * log10f(1.0f + hz / 700.0f);
}

// Convert mel scale to frequency
static float mel_to_hz(float mel) {
    return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
}

// Initialize mel filterbank
static void init_mel_filterbank(void) {
    memset(mel_filterbank, 0, sizeof(mel_filterbank));
    
    float mel_min = hz_to_mel(0);
    float mel_max = hz_to_mel(AUDIO_SAMPLE_RATE / 2.0f);
    float mel_step = (mel_max - mel_min) / (AUDIO_MEL_BANDS + 1);
    
    for (int i = 0; i < AUDIO_MEL_BANDS; i++) {
        float mel_center = mel_min + (i + 1) * mel_step;
        float mel_left = mel_min + i * mel_step;
        float mel_right = mel_min + (i + 2) * mel_step;
        
        float hz_center = mel_to_hz(mel_center);
        float hz_left = mel_to_hz(mel_left);
        float hz_right = mel_to_hz(mel_right);
        
        int bin_left = (int)(hz_left * AUDIO_FFT_SIZE / AUDIO_SAMPLE_RATE);
        int bin_center = (int)(hz_center * AUDIO_FFT_SIZE / AUDIO_SAMPLE_RATE);
        int bin_right = (int)(hz_right * AUDIO_FFT_SIZE / AUDIO_SAMPLE_RATE);
        
        // Create triangular filter
        for (int j = bin_left; j < bin_center && j < AUDIO_FFT_SIZE / 2; j++) {
            mel_filterbank[i][j] = (float)(j - bin_left) / (bin_center - bin_left);
        }
        for (int j = bin_center; j < bin_right && j < AUDIO_FFT_SIZE / 2; j++) {
            mel_filterbank[i][j] = (float)(bin_right - j) / (bin_right - bin_center);
        }
    }
}

// Apply mel filterbank and compute band energies
static void compute_mel_spectrogram(float32_t *fft_mag) {
    for (int i = 0; i < AUDIO_MEL_BANDS; i++) {
        float energy = 0.0f;
        for (int j = 0; j < AUDIO_FFT_SIZE / 2; j++) {
            energy += fft_mag[j] * mel_filterbank[i][j];
        }
        
        // Convert to dB scale and normalize to full 0-255 range
        float db = 10.0f * log10f(energy + 1.0f);
        db = (db - 27.0f) / 21.0f * 255.0f;  // Map 27-48dB range to 0-255
        
        if (db < 0) db = 0;
        if (db > 255) db = 255;
        
        mel_values[i] = (uint8_t)db;
        
        // Apply smoothing
        mel_values_smoothed[i] = (uint8_t)(
            SMOOTHING_FACTOR * mel_values_smoothed[i] +
            (1.0f - SMOOTHING_FACTOR) * mel_values[i]
        );
    }
}

// Detect beats based on bass energy
static void detect_beat(void) {
    uint8_t current_bass = audio_viz_get_bass();
    
    // Calculate average of recent bass levels
    uint32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += bass_history[i];
    }
    uint8_t avg_bass = sum / 8;
    
    // Beat detected if current bass significantly exceeds average
    beat_detected = (current_bass > avg_bass + 40);
    
    // Update history
    bass_history[bass_history_idx] = current_bass;
    bass_history_idx = (bass_history_idx + 1) % 8;
}

// Process audio buffer with FFT
static void process_audio_buffer(int16_t *buffer, size_t sample_count) {
    if (!buffer || sample_count == 0) {
        return;
    }
    
    // Copy samples to FFT input buffer (zero-pad if needed)
    size_t copy_count = (sample_count < AUDIO_FFT_SIZE) ? sample_count : AUDIO_FFT_SIZE;
    
    for (size_t i = 0; i < copy_count; i++) {
        fft_input[i] = (float32_t)buffer[i];
    }
    for (size_t i = copy_count; i < AUDIO_FFT_SIZE; i++) {
        fft_input[i] = 0.0f;
    }
    
    // Perform FFT
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);
    
    // Compute magnitude spectrum
    float32_t magnitudes[AUDIO_FFT_SIZE / 2];
    for (int i = 0; i < AUDIO_FFT_SIZE / 2; i++) {
        float real = fft_output[i * 2];
        float imag = fft_output[i * 2 + 1];
        magnitudes[i] = sqrtf(real * real + imag * imag);
    }
    
    // Compute mel spectrogram
    compute_mel_spectrogram(magnitudes);
    
    // Detect beats
    detect_beat();
}

int audio_viz_init(void) {
    LOG_INF("Initializing audio visualizer...");
    
    // Initialize FFT
    arm_rfft_fast_init_f32(&fft_instance, AUDIO_FFT_SIZE);
    
    // Initialize mel filterbank
    init_mel_filterbank();
    
    // Clear buffers
    memset(mel_values, 0, sizeof(mel_values));
    memset(mel_values_smoothed, 0, sizeof(mel_values_smoothed));
    memset(bass_history, 0, sizeof(bass_history));
    
    // Get DMIC device
    dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));
    if (!device_is_ready(dmic_dev)) {
        LOG_WRN("DMIC device not ready");
        return -ENODEV;
    }
    
    audio_available = true;
    LOG_INF("Audio visualizer initialized");
    return 0;
}

int audio_viz_start(void) {
    if (!audio_available) {
        return -ENODEV;
    }
    
    if (audio_running) {
        return 0; // Already running
    }
    
    // Configure DMIC
    struct pcm_stream_cfg stream = {
        .pcm_width = SAMPLE_BIT_WIDTH,
        .mem_slab  = &audio_mem_slab,
    };
    
    struct dmic_cfg cfg = {
        .io = {
            .min_pdm_clk_freq = 1000000,
            .max_pdm_clk_freq = 3500000,
            .min_pdm_clk_dc   = 40,
            .max_pdm_clk_dc   = 60,
        },
        .streams = &stream,
        .channel = {
            .req_num_streams = 1,
            .req_num_chan = 1,
        },
    };
    
    cfg.channel.req_chan_map_lo = dmic_build_channel_map(0, 0, PDM_CHAN_LEFT);
    cfg.streams[0].pcm_rate = AUDIO_SAMPLE_RATE;
    cfg.streams[0].block_size = BLOCK_SIZE(cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);
    
    int ret = dmic_configure(dmic_dev, &cfg);
    if (ret < 0) {
        LOG_ERR("Failed to configure DMIC: %d", ret);
        return ret;
    }
    
    ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
    if (ret < 0) {
        LOG_ERR("Failed to start DMIC: %d", ret);
        return ret;
    }
    
    audio_running = true;
    LOG_INF("Audio capture started");
    return 0;
}

void audio_viz_stop(void) {
    if (audio_running && audio_available) {
        dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
        audio_running = false;
        LOG_INF("Audio capture stopped");
    }
}

bool audio_viz_available(void) {
    return audio_available && audio_running;
}

void audio_viz_process(void) {
    if (!audio_viz_available()) {
        return;
    }
    
    void *buffer;
    uint32_t size;
    
    int ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
    if (ret < 0) {
        if (ret != -EAGAIN) {
            LOG_ERR("DMIC read failed: %d", ret);
        }
        return;
    }
    
    // Process audio
    size_t sample_count = size / BYTES_PER_SAMPLE;
    process_audio_buffer((int16_t *)buffer, sample_count);
    
    // Log band values for tuning
    static uint32_t log_counter = 0;
    if (++log_counter % 10 == 0) {  // Log every 10th frame (~1 second)
        LOG_INF("Bands: %3d %3d %3d %3d %3d %3d %3d %3d | Bass:%3d Mid:%3d High:%3d Vol:%3d Beat:%d",
                mel_values_smoothed[0], mel_values_smoothed[1], mel_values_smoothed[2], mel_values_smoothed[3],
                mel_values_smoothed[4], mel_values_smoothed[5], mel_values_smoothed[6], mel_values_smoothed[7],
                audio_viz_get_bass(), audio_viz_get_mids(), audio_viz_get_highs(), 
                audio_viz_get_volume(), beat_detected ? 1 : 0);
    }
    
    k_mem_slab_free(&audio_mem_slab, buffer);
}

uint8_t audio_viz_get_band(int band) {
    if (band < 0 || band >= AUDIO_MEL_BANDS) {
        return 0;
    }
    return mel_values_smoothed[band];
}

uint8_t audio_viz_get_bass(void) {
    uint32_t sum = 0;
    for (int i = 0; i < AUDIO_LOW_BANDS; i++) {
        sum += mel_values_smoothed[i];
    }
    return sum / AUDIO_LOW_BANDS;
}

uint8_t audio_viz_get_mids(void) {
    uint32_t sum = 0;
    for (int i = AUDIO_LOW_BANDS; i < AUDIO_LOW_BANDS + AUDIO_MID_BANDS; i++) {
        sum += mel_values_smoothed[i];
    }
    return sum / AUDIO_MID_BANDS;
}

uint8_t audio_viz_get_highs(void) {
    uint32_t sum = 0;
    for (int i = AUDIO_LOW_BANDS + AUDIO_MID_BANDS; i < AUDIO_MEL_BANDS; i++) {
        sum += mel_values_smoothed[i];
    }
    return sum / AUDIO_HIGH_BANDS;
}

uint8_t audio_viz_get_volume(void) {
    uint32_t sum = 0;
    for (int i = 0; i < AUDIO_MEL_BANDS; i++) {
        sum += mel_values_smoothed[i];
    }
    return sum / AUDIO_MEL_BANDS;
}

bool audio_viz_beat_detected(void) {
    return beat_detected;
}
