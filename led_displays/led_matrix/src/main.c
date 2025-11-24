/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 TOKITA Hiroshi
 * Copyright (c) 2025 LED Matrix Adaptation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/util.h>
#include <hal/nrf_comp.h>

#include "font_5x5.h"
#include "water_physics.h"
#include "audio_viz.h"
#include "animations.h"


#define STRIP_NODE		DT_ALIAS(led_strip)

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

// Matrix dimensions from Kconfig
#define MATRIX_WIDTH	40  // 5 matrices × 8 pixels wide
#define MATRIX_HEIGHT	6
#define MATRIX_SIZE	(MATRIX_WIDTH * MATRIX_HEIGHT)

#if STRIP_NUM_PIXELS != MATRIX_SIZE
#error "LED strip chain length must match matrix size (WIDTH * HEIGHT)"
#endif

#define DELAY_TIME K_MSEC(CONFIG_SAMPLE_LED_UPDATE_DELAY)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

// LSM6DSO I2C definitions
#define LSM6DSO_I2C_ADDR    0x6A
#define LSM6DSO_REG_WHO_AM_I 0x0F
#define LSM6DSO_WHO_AM_I_VAL 0x6A
#define LSM6DSO_REG_CTRL1_XL 0x10
#define LSM6DSO_REG_CTRL2_G  0x11
#define LSM6DSO_REG_OUTX_L_XL 0x28

static struct led_rgb pixels[STRIP_NUM_PIXELS];
static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
static const struct device *i2c_dev;

// Tilt state for liquid simulation
static float tilt_x = 0.0f;  // Roll (left/right)
static float tilt_y = 0.0f;  // Pitch (forward/backward)
static bool imu_available = false;

// Animation mode state
static bool animation_mode = false;  // false = water sim, true = audio/IMU animations
static uint32_t last_frame_time = 0;

// Button state for animation control
static bool button_pressed = false;

// Button device for animation control
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

/**
 * @brief Convert x,y matrix coordinates to linear pixel index
 * Each of 5 matrices is 8×6 pixels (48 LEDs) in its own raster format
 * Matrices are chained: Matrix 0 (pixels 0-47), Matrix 1 (48-95), etc.
 * @param x Column (0 to MATRIX_WIDTH-1, spans 5 matrices of 8 pixels each)
 * @param y Row (0 to MATRIX_HEIGHT-1, same for all matrices)
 * @return Linear index for the chained LED strip
 */
static inline size_t xy_to_index(int x, int y)
{
	if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
		return MATRIX_SIZE; // Out of bounds
	}
	
	// Determine which 8×6 matrix this pixel belongs to
	int matrix_num = x / 8;      // Which matrix (0-4)
	int local_x = x % 8;         // X position within that matrix (0-7)
	int local_y = y;             // Y position (0-5, same for all matrices)
	
	// Each matrix has 48 LEDs in raster format (row 0: 0-7, row 1: 8-15, etc.)
	int matrix_offset = matrix_num * 48;
	int pixel_in_matrix = local_y * 8 + local_x;
	
	return matrix_offset + pixel_in_matrix;
}

/**
 * @brief Set a single pixel in the matrix
 * @param x Column position
 * @param y Row position
 * @param color RGB color value
 */
static void set_pixel(int x, int y, struct led_rgb color)
{
	size_t idx = xy_to_index(x, y);
	if (idx < MATRIX_SIZE) {
		memcpy(&pixels[idx], &color, sizeof(struct led_rgb));
	}
}

/**
 * @brief Clear all pixels (set to black)
 */
static void clear_matrix(void)
{
	memset(pixels, 0, sizeof(pixels));
}

/**
 * @brief Fill entire matrix with one color
 */
static void fill_matrix(struct led_rgb color)
{
	for (size_t i = 0; i < MATRIX_SIZE; i++) {
		memcpy(&pixels[i], &color, sizeof(struct led_rgb));
	}
}

/**
 * @brief Draw a horizontal line
 */
static void draw_hline(int x, int y, int width, struct led_rgb color)
{
	for (int i = 0; i < width; i++) {
		set_pixel(x + i, y, color);
	}
}

/**
 * @brief Draw a vertical line
 */
static void draw_vline(int x, int y, int height, struct led_rgb color)
{
	for (int i = 0; i < height; i++) {
		set_pixel(x, y + i, color);
	}
}

/**
 * @brief Draw a rectangle outline
 */
__attribute__((unused))
static void draw_rect(int x, int y, int w, int h, struct led_rgb color)
{
	draw_hline(x, y, w, color);
	draw_hline(x, y + h - 1, w, color);
	draw_vline(x, y, h, color);
	draw_vline(x + w - 1, y, h, color);
}

/**
 * @brief Fill a rectangle
 */
__attribute__((unused))
static void fill_rect(int x, int y, int w, int h, struct led_rgb color)
{
	for (int row = 0; row < h; row++) {
		draw_hline(x, y + row, w, color);
	}
}

/**
 * @brief Update the physical LED strip from the pixel buffer
 */
static int update_display(void)
{
	return led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
}

/**
 * @brief Initialize LSM6DSO via I2C
 */
static int lsm6dso_init(void)
{
	uint8_t who_am_i = 0;
	int ret;

	// Verify device ID
	ret = i2c_reg_read_byte(i2c_dev, LSM6DSO_I2C_ADDR, LSM6DSO_REG_WHO_AM_I, &who_am_i);
	if (ret != 0) {
		LOG_ERR("Failed to read WHO_AM_I (err: %d)", ret);
		return ret;
	}
	if (who_am_i != LSM6DSO_WHO_AM_I_VAL) {
		LOG_ERR("Invalid WHO_AM_I: 0x%02x", who_am_i);
		return -ENODEV;
	}
	LOG_INF("LSM6DSO found, ID: 0x%02x", who_am_i);

	// Set accelerometer ODR (12.5 Hz) and 2g range
	uint8_t tx_buf[2] = {LSM6DSO_REG_CTRL1_XL, 0x20};
	ret = i2c_write(i2c_dev, tx_buf, 2, LSM6DSO_I2C_ADDR);
	if (ret != 0) {
		LOG_ERR("Failed to set CTRL1_XL (err: %d)", ret);
		return ret;
	}

	// Set gyroscope ODR (12.5 Hz) and 250dps range
	tx_buf[0] = LSM6DSO_REG_CTRL2_G;
	tx_buf[1] = 0x20;
	ret = i2c_write(i2c_dev, tx_buf, 2, LSM6DSO_I2C_ADDR);
	if (ret != 0) {
		LOG_ERR("Failed to set CTRL2_G (err: %d)", ret);
		return ret;
	}

	LOG_INF("LSM6DSO initialized");
	return 0;
}

/**
 * @brief Read IMU and calculate tilt angles
 */
static int read_imu(void)
{
	uint8_t accel_data[6];
	int ret;

	if (!imu_available) {
		return -ENODEV;
	}

	// Read accelerometer data (6 bytes)
	ret = i2c_burst_read(i2c_dev, LSM6DSO_I2C_ADDR, LSM6DSO_REG_OUTX_L_XL, accel_data, 6);
	if (ret != 0) {
		return ret;
	}

	// Convert raw data to signed integers (low byte first)
	int16_t raw_x = (int16_t)(accel_data[0] | (accel_data[1] << 8));
	int16_t raw_y = (int16_t)(accel_data[2] | (accel_data[3] << 8));
	int16_t raw_z = (int16_t)(accel_data[4] | (accel_data[5] << 8));

	// Convert to g (assuming ±2g range, sensitivity ~0.061 mg/LSB)
	float acc_x = raw_x * 0.000061f * 9.81f; // Convert to m/s²
	float acc_y = raw_y * 0.000061f * 9.81f;
	float acc_z = raw_z * 0.000061f * 9.81f;

	// Calculate tilt angles (in degrees)
	tilt_x = atan2f(acc_x, acc_z) * 180.0f / 3.14159f;
	tilt_y = atan2f(acc_y, acc_z) * 180.0f / 3.14159f;

	return 0;
}

/**
 * @brief Initialize button for animation control
 */
static int init_button(void)
{
	if (!gpio_is_ready_dt(&button)) {
		LOG_ERR("Button device not ready");
		return -ENODEV;
	}

	int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Failed to configure button: %d", ret);
		return ret;
	}

	LOG_INF("Button initialized for animation control");
	return 0;
}

/**
 * @brief Check button state and handle animation cycling
 */
static void check_button(void)
{
	bool current_state = gpio_pin_get_dt(&button);
	
	// Button pressed (active low)
	if (!current_state && !button_pressed) {
		button_pressed = true;
		
		// Cycle to next animation
		if (animation_mode) {
			animations_next_mode();
			LOG_INF("Animation: %s", 
				animations_get_mode_name(animations_get_mode()));
		}
	} else if (current_state) {
		button_pressed = false;
	}
}

/**
 * @brief Draw a character at position using 5x5 font
 */
static void draw_char(int x, int y, char c, struct led_rgb color)
{
	// Convert lowercase to uppercase
	if (c >= 'a' && c <= 'z') {
		c = c - 'a' + 'A';
	}
	
	// Check if character is in our font range (space to 'Z')
	if (c < ' ' || c > 'Z') {
		c = ' '; // Default to space for unsupported chars
	}
	
	const uint8_t *glyph = font_5x5[c - ' '];
	
	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 5; col++) {
			if (glyph[row] & (1 << (4 - col))) {
				set_pixel(x + col, y + row, color);
			}
		}
	}
}

/**
 * @brief Draw a string with 6x5 character cells (5px char + 1px space)
 */
static void draw_string(int x, int y, const char *str, struct led_rgb color)
{
	int cursor_x = x;
	while (*str) {
		draw_char(cursor_x, y, *str, color);
		cursor_x += 6; // 5 pixels for char + 1 pixel spacing
		str++;
	}
}

/**
 * @brief Get pixel width of a string
 */
__attribute__((unused))
static int string_width(const char *str)
{
	int len = 0;
	while (*str++) len++;
	if (len == 0) return 0;
	return len * 6 - 1; // 6 pixels per char, minus last space
}

/**
 * @brief Demo: Scrolling text
 */
__attribute__((unused))
static void demo_scrolling_text(int offset, const char *text, struct led_rgb color)
{
	clear_matrix();
	int x = MATRIX_WIDTH - offset;
	
	// Draw text centered vertically
	draw_string(x, 1, text, color);
}

/**
 * @brief Render liquid on LED matrix using water_physics module
 */
static void render_liquid(struct led_rgb color)
{
	clear_matrix();
	
	for (int y = 0; y < MATRIX_HEIGHT; y++) {
		for (int x = 0; x < MATRIX_WIDTH; x++) {
			if (water_physics_get_cell(x, y) == 1) {
				set_pixel(x, y, color);
			}
		}
	}
}

int main(void)
{
	int rc;

	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

	// Initialize I2C and IMU
	i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c30));
	if (device_is_ready(i2c_dev)) {
		LOG_INF("I2C device ready");
		if (lsm6dso_init() == 0) {
			imu_available = true;
			printf("IMU initialized successfully\n");
		} else {
			LOG_WRN("IMU initialization failed, continuing without IMU");
			printf("IMU not available\n");
		}
	} else {
		LOG_WRN("I2C device not ready");
	}

	// Initialize button for controls
	if (init_button() != 0) {
		LOG_WRN("Button initialization failed, continuing without button");
	}

	// Initialize audio visualizer
	if (audio_viz_init() == 0) {
		LOG_INF("Audio visualizer initialized");
	} else {
		LOG_WRN("Audio visualizer not available");
	}

	// Initialize animations
	animations_init(MATRIX_WIDTH, MATRIX_HEIGHT);
	LOG_INF("Animations initialized");

	LOG_INF("LED Matrix: %dx%d (%d pixels) in raster format", 
		MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_SIZE);
	LOG_INF("Max power at brightness 105: ~1435 mA (safe for 5 matrices @ 1.5A)");
	LOG_INF("Current brightness: %d", CONFIG_SAMPLE_LED_BRIGHTNESS);
	
	// Flash all LEDs briefly to confirm connection
	fill_matrix((struct led_rgb)RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, 0, 0));
	update_display();
	k_msleep(200);
	
	fill_matrix((struct led_rgb)RGB(0, CONFIG_SAMPLE_LED_BRIGHTNESS, 0));
	update_display();
	k_msleep(200);
	
	fill_matrix((struct led_rgb)RGB(0, 0, CONFIG_SAMPLE_LED_BRIGHTNESS));
	update_display();
	k_msleep(200);
	
	clear_matrix();
	update_display();
	k_msleep(500);

	// Initialize water physics simulation with "HELLO" text
	if (water_physics_init_text(MATRIX_WIDTH, MATRIX_HEIGHT, "HELLO") != 0) {
		LOG_ERR("Failed to initialize water physics");
		return 0;
	}
	LOG_INF("Water physics initialized with HELLO text");
	
	// Cyan/blue water color (much dimmer)
	struct led_rgb water_color = {.r = 0, .g = 20, .b = 40};
	
	// Start in animation mode
	animation_mode = true;
	if (audio_viz_start() == 0) {
		LOG_INF("Audio capture started");
	}
	last_frame_time = k_uptime_get_32();
	
	LOG_INF("==============================================");
	LOG_INF("Button Controls:");
	LOG_INF("  SW0 (User Button) - Cycle through animations");
	LOG_INF("==============================================");
	
	int frame_counter = 0;
	bool first_frame = true;
	
	while (1) {
		uint32_t current_time = k_uptime_get_32();
		uint32_t delta_time = current_time - last_frame_time;
		last_frame_time = current_time;
		
		// Read IMU every frame
		if (imu_available) {
			read_imu();
		}

		// Check button for animation cycling
		check_button();
		
		if (animation_mode) {
			// Animation mode - process audio and render animations
			audio_viz_process();
			animations_update(pixels, tilt_x, tilt_y, delta_time);
		} else {
			// Water simulation mode
			if (first_frame) {
				render_liquid(water_color);
				update_display();
				first_frame = false;
			}
			
			water_physics_set_tilt(tilt_x, tilt_y);
			water_physics_update();
			render_liquid(water_color);
		}
		
		rc = update_display();
		if (rc) {
			LOG_ERR("Couldn't update display: %d", rc);
		}
		
		frame_counter++;
		// No sleep - run as fast as possible, delta_time handles frame rate
	}

	return 0;
}
