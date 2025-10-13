/*
 * Audio Visualizer for XIAO nRF54L15
 * Captures audio from DMIC, performs FFT, and displays mel-spectrogram on OLED
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/logging/log.h>

// CMSIS-DSP for FFT
#include <arm_math.h>

// U8g2 for OLED display
#include "../../../lib/u8g2/csrc/u8g2.h"
#include "../../../lib/u8g2/csrc/u8x8.h"

LOG_MODULE_REGISTER(audio_viz, CONFIG_LOG_DEFAULT_LEVEL);

// ===================================================================================
// Audio Configuration
// ===================================================================================
#define SAMPLE_RATE      16000
#define SAMPLE_BIT_WIDTH 16
#define BYTES_PER_SAMPLE sizeof(int16_t)
#define READ_TIMEOUT     1000

// FFT Configuration
#define FFT_SIZE         512  // Must be power of 2
#define FFT_HALF         (FFT_SIZE / 2)
#define BLOCK_SIZE(_sample_rate, _number_of_channels) \
	(BYTES_PER_SAMPLE * (_sample_rate / 10) * _number_of_channels)

#define MAX_BLOCK_SIZE   BLOCK_SIZE(SAMPLE_RATE, 1)
#define BLOCK_COUNT      4
K_MEM_SLAB_DEFINE_STATIC(mem_slab, MAX_BLOCK_SIZE, BLOCK_COUNT, 4);

// Mel-spectrogram Configuration
#define MEL_BANDS        32  // Number of mel frequency bands to display
#define DISPLAY_WIDTH    128
#define DISPLAY_HEIGHT   64

// ===================================================================================
// GPIO Configuration
// ===================================================================================
static const struct gpio_dt_spec led_builtin = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// ===================================================================================
// I2C and Display Configuration
// ===================================================================================
const struct device *i2c22_dev = DEVICE_DT_GET(DT_NODELABEL(i2c22));

#define I2C_BUFFER_SIZE 64
static uint8_t i2c_buf[I2C_BUFFER_SIZE];
static uint8_t i2c_buf_len = 0;

uint8_t u8x8_byte_zephyr_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {    
    switch (msg) {
    case U8X8_MSG_BYTE_INIT:
        return 1;
    case U8X8_MSG_BYTE_START_TRANSFER:
        i2c_buf_len = 0;
        return 1;
    case U8X8_MSG_BYTE_SEND:
        if (i2c_buf_len + arg_int > I2C_BUFFER_SIZE)
            return 0;
        memcpy(&i2c_buf[i2c_buf_len], arg_ptr, arg_int);
        i2c_buf_len += arg_int;
        return 1;
    case U8X8_MSG_BYTE_END_TRANSFER:
        i2c_write(i2c22_dev, i2c_buf, i2c_buf_len,
                  u8x8_GetI2CAddress(u8x8) >> 1);
        return 1;
    }
    return 0;
}

uint8_t u8x8_gpio_and_delay_zephyr(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        k_sleep(K_MSEC(1));
        break;
    case U8X8_MSG_DELAY_MILLI:
        k_sleep(K_MSEC(arg_int));
        break;
    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
        break;
    }
    return 1;
}

static u8g2_t u8g2;

// ===================================================================================
// FFT and Audio Processing
// ===================================================================================
static float32_t fft_input[FFT_SIZE * 2];  // Complex input (real + imaginary)
static float32_t fft_output[FFT_SIZE];     // Magnitude output
static arm_rfft_fast_instance_f32 fft_instance;

// Mel filter bank (simplified - triangular filters)
static float mel_filterbank[MEL_BANDS][FFT_HALF];
static uint8_t mel_values[MEL_BANDS];

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
    float mel_max = hz_to_mel(SAMPLE_RATE / 2.0f);
    float mel_step = (mel_max - mel_min) / (MEL_BANDS + 1);
    
    for (int i = 0; i < MEL_BANDS; i++) {
        float mel_center = mel_min + (i + 1) * mel_step;
        float mel_left = mel_min + i * mel_step;
        float mel_right = mel_min + (i + 2) * mel_step;
        
        float hz_center = mel_to_hz(mel_center);
        float hz_left = mel_to_hz(mel_left);
        float hz_right = mel_to_hz(mel_right);
        
        int bin_left = (int)(hz_left * FFT_SIZE / SAMPLE_RATE);
        int bin_center = (int)(hz_center * FFT_SIZE / SAMPLE_RATE);
        int bin_right = (int)(hz_right * FFT_SIZE / SAMPLE_RATE);
        
        // Create triangular filter
        for (int j = bin_left; j < bin_center && j < FFT_HALF; j++) {
            mel_filterbank[i][j] = (float)(j - bin_left) / (bin_center - bin_left);
        }
        for (int j = bin_center; j < bin_right && j < FFT_HALF; j++) {
            mel_filterbank[i][j] = (float)(bin_right - j) / (bin_right - bin_center);
        }
    }
}

// Apply mel filterbank and compute band energies
static void compute_mel_spectrogram(float32_t *fft_mag) {
    for (int i = 0; i < MEL_BANDS; i++) {
        float energy = 0.0f;
        for (int j = 0; j < FFT_HALF; j++) {
            energy += fft_mag[j] * mel_filterbank[i][j];
        }
        
        // Convert to dB scale and normalize with wider dynamic range
        float db = 10.0f * log10f(energy + 1.0f);
        db = (db - 20.0f) / 80.0f * 255.0f;  // Wider range, higher threshold
        
        if (db < 0) db = 0;
        if (db > 255) db = 255;
        
        mel_values[i] = (uint8_t)db;
    }
}

// Draw spectrogram on OLED
static void draw_spectrogram(void) {
    u8g2_ClearBuffer(&u8g2);
    
    // Draw title
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
    u8g2_DrawStr(&u8g2, 0, 7, "Audio Visualizer");
    
    // Draw mel bands as vertical bars
    int bar_width = DISPLAY_WIDTH / MEL_BANDS;
    int max_height = DISPLAY_HEIGHT - 10;
    
    for (int i = 0; i < MEL_BANDS; i++) {
        int height = (mel_values[i] * max_height) / 255;
        int x = i * bar_width;
        int y = DISPLAY_HEIGHT - height;
        
        u8g2_DrawBox(&u8g2, x, y, bar_width - 1, height);
    }
    
    u8g2_SendBuffer(&u8g2);
}

// Process audio buffer with FFT
static void process_audio(int16_t *buffer, size_t sample_count) {
    // Copy samples to FFT input buffer (zero-pad if needed)
    size_t copy_count = (sample_count < FFT_SIZE) ? sample_count : FFT_SIZE;
    
    for (size_t i = 0; i < copy_count; i++) {
        fft_input[i] = (float32_t)buffer[i];
    }
    for (size_t i = copy_count; i < FFT_SIZE; i++) {
        fft_input[i] = 0.0f;
    }
    
    // Perform FFT
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);
    
    // Compute magnitude spectrum
    float32_t magnitudes[FFT_HALF];
    for (int i = 0; i < FFT_HALF; i++) {
        float real = fft_output[i * 2];
        float imag = fft_output[i * 2 + 1];
        magnitudes[i] = sqrtf(real * real + imag * imag);
    }
    
    // Compute mel spectrogram
    compute_mel_spectrogram(magnitudes);
    
    // Draw on display
    draw_spectrogram();
}

// ===================================================================================
// Main Application
// ===================================================================================
int main(void) {
    LOG_INF("Audio Visualizer Starting...");
    
    // Initialize LED
    if (!gpio_is_ready_dt(&led_builtin)) {
        LOG_ERR("LED device not ready");
        return -1;
    }
    gpio_pin_configure_dt(&led_builtin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set_dt(&led_builtin, 1);  // LED on during init
    
    // Initialize I2C for display
    if (!device_is_ready(i2c22_dev)) {
        LOG_ERR("I2C device not ready");
        return -1;
    }
    
    // Initialize U8g2 display
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0,
        u8x8_byte_zephyr_hw_i2c, u8x8_gpio_and_delay_zephyr);
    u8g2_SetI2CAddress(&u8g2, 0x3C << 1);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearDisplay(&u8g2);
    
    // Display startup message with branding
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 8, 20, "XIAO nRF54L15");
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 10, 38, "Audio Vis");
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
    u8g2_DrawStr(&u8g2, 20, 55, "Initializing...");
    u8g2_SendBuffer(&u8g2);
    k_sleep(K_MSEC(2000));
    
    // Initialize FFT
    arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);
    
    // Initialize mel filterbank
    init_mel_filterbank();
    
    // Initialize DMIC
    const struct device *const dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));
    if (!device_is_ready(dmic_dev)) {
        LOG_ERR("DMIC device not ready");
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
        u8g2_DrawStr(&u8g2, 10, 30, "DMIC Error!");
        u8g2_SendBuffer(&u8g2);
        return -1;
    }
    
    // Configure DMIC
    struct pcm_stream_cfg stream = {
        .pcm_width = SAMPLE_BIT_WIDTH,
        .mem_slab  = &mem_slab,
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
    cfg.streams[0].pcm_rate = SAMPLE_RATE;
    cfg.streams[0].block_size = BLOCK_SIZE(cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);
    
    int ret = dmic_configure(dmic_dev, &cfg);
    if (ret < 0) {
        LOG_ERR("Failed to configure DMIC: %d", ret);
        return -1;
    }
    
    ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
    if (ret < 0) {
        LOG_ERR("Failed to start DMIC: %d", ret);
        return -1;
    }
    
    gpio_pin_set_dt(&led_builtin, 0);  // LED off - ready
    LOG_INF("Audio capture started!");
    
    // Main loop - capture audio and visualize
    while (1) {
        void *buffer;
        uint32_t size;
        
        ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
        if (ret < 0) {
            LOG_ERR("DMIC read failed: %d", ret);
            k_sleep(K_MSEC(100));
            continue;
        }
        
        // Process audio and update display
        size_t sample_count = size / BYTES_PER_SAMPLE;
        process_audio((int16_t *)buffer, sample_count);
        
        k_mem_slab_free(&mem_slab, buffer);
    }
    
    return 0;
}
