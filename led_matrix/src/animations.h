/*
 * LED Matrix Animations - Audio and IMU reactive effects
 */

#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <zephyr/drivers/led_strip.h>
#include <stdint.h>
#include <stdbool.h>

// Animation mode enum
typedef enum {
    ANIM_TEST_MODE,          // TEST: Column numbers and IMU direction indicators
    ANIM_VU_METER,           // VU meter with peak hold (40 columns, green to red)
    ANIM_SPECTRUM_BARS,      // Vertical frequency bars (rainbow gradient)
    ANIM_PULSE,              // Pulsing circle based on bass
    ANIM_WAVEFORM,           // Scrolling waveform
    ANIM_GRAVITY_PARTICLES,  // Particles affected by tilt
    ANIM_TILT_GRADIENT,      // Color gradient following tilt
    ANIM_SHAKE_BURST,        // Burst effect on shake
    ANIM_AUDIO_RAIN,         // Sound-reactive falling particles
    ANIM_BASS_RIPPLE,        // Expanding ripples on bass hits
    ANIM_REACTIVE_SPIRAL,    // Spiral that responds to audio+tilt
    ANIM_MODE_COUNT
} animation_mode_t;

// Matrix dimensions
#define ANIM_WIDTH  40
#define ANIM_HEIGHT 6

/**
 * @brief Initialize animations system
 * @param width Matrix width
 * @param height Matrix height
 */
void animations_init(int width, int height);

/**
 * @brief Update current animation
 * @param pixels Output pixel buffer
 * @param tilt_x Roll angle (-90 to +90 degrees)
 * @param tilt_y Pitch angle (-90 to +90 degrees)
 * @param delta_time Time since last update in ms
 */
void animations_update(struct led_rgb *pixels, float tilt_x, float tilt_y, uint32_t delta_time);

/**
 * @brief Set current animation mode
 * @param mode Animation mode to set
 */
void animations_set_mode(animation_mode_t mode);

/**
 * @brief Get current animation mode
 * @return Current animation mode
 */
animation_mode_t animations_get_mode(void);

/**
 * @brief Cycle to next animation mode
 */
void animations_next_mode(void);

/**
 * @brief Get name of animation mode
 * @param mode Animation mode
 * @return Name string
 */
const char* animations_get_mode_name(animation_mode_t mode);

#endif // ANIMATIONS_H
