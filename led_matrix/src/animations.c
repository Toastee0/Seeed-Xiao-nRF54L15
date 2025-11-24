/*
 * LED Matrix Animations - Audio and IMU reactive effects
 */

#include "animations.h"
#include "audio_viz.h"
#include <string.h>
#include <math.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

// Current state
static animation_mode_t current_mode = ANIM_TEST_MODE;
static int matrix_width = 40;
static int matrix_height = 6;
static bool animations_ready = false;

// Particle system (for gravity and rain effects)
#define MAX_PARTICLES 40
typedef struct {
    float x, y;
    float vx, vy;
    uint8_t life;
    struct led_rgb color;
} particle_t;
static particle_t particles[MAX_PARTICLES];

// Ripple system (for bass ripples)
#define MAX_RIPPLES 4
typedef struct {
    float x, y;
    float radius;
    uint8_t intensity;
    bool active;
} ripple_t;
static ripple_t ripples[MAX_RIPPLES];

// Peak hold for VU meter (now 40 columns instead of 8)
static uint8_t peak_hold[40];
static uint32_t peak_hold_time[40];

// Waveform history
#define WAVEFORM_HISTORY 40
static uint8_t waveform_buffer[WAVEFORM_HISTORY];
static uint8_t waveform_idx = 0;

// Shake detection
static float prev_tilt_x = 0, prev_tilt_y = 0;
static bool shake_active = false;
static uint32_t shake_time = 0;

// Helper: Convert x,y to linear index (same as main.c)
static inline size_t xy_to_index(int x, int y) {
    if (x < 0 || x >= matrix_width || y < 0 || y >= matrix_height) {
        return matrix_width * matrix_height;
    }
    int matrix_num = x / 8;
    int local_x = x % 8;
    int local_y = y;
    int matrix_offset = matrix_num * 48;
    int pixel_in_matrix = local_y * 8 + local_x;
    return matrix_offset + pixel_in_matrix;
}

// Helper: Set pixel with bounds checking
static void set_pixel(struct led_rgb *pixels, int x, int y, struct led_rgb color) {
    size_t idx = xy_to_index(x, y);
    if (idx < (size_t)(matrix_width * matrix_height)) {
        pixels[idx] = color;
    }
}

// Helper: Clear all pixels
static void clear_pixels(struct led_rgb *pixels) {
    memset(pixels, 0, matrix_width * matrix_height * sizeof(struct led_rgb));
}

// ============================================================================
// TEST MODE: Show column numbers and IMU tilt directions
// ============================================================================
static void anim_test_mode(struct led_rgb *pixels, float tilt_x, float tilt_y) {
    clear_pixels(pixels);
    
    // Show column indicators every 5 columns (columns 0, 5, 10, 15, 20, 25, 30, 35)
    for (int x = 0; x < matrix_width; x += 5) {
        // Make column marker bright white (full 6 pixels tall)
        for (int y = 0; y < matrix_height; y++) {
            set_pixel(pixels, x, y, (struct led_rgb){60, 60, 60});
        }
    }
    
    // Column 0 is RED (far left)
    for (int y = 0; y < matrix_height; y++) {
        set_pixel(pixels, 0, y, (struct led_rgb){60, 0, 0});
    }
    
    // Column 39 is BLUE (far right)
    for (int y = 0; y < matrix_height; y++) {
        set_pixel(pixels, matrix_width - 1, y, (struct led_rgb){0, 0, 60});
    }
    
    // Center column (20) is GREEN
    for (int y = 0; y < matrix_height; y++) {
        set_pixel(pixels, matrix_width / 2, y, (struct led_rgb){0, 60, 0});
    }
    
    // IMU tilt_x indicator: positive tilt = cyan dot moves RIGHT
    // Map tilt_x range (-45 to +45) to x position (5 to 35)
    float tilt_x_clamped = tilt_x;
    if (tilt_x_clamped < -45.0f) tilt_x_clamped = -45.0f;
    if (tilt_x_clamped > 45.0f) tilt_x_clamped = 45.0f;
    int x_pos = (int)((tilt_x_clamped + 45.0f) * 30.0f / 90.0f) + 5;
    if (x_pos >= 0 && x_pos < matrix_width) {
        set_pixel(pixels, x_pos, 2, (struct led_rgb){0, 60, 60});  // Cyan dot at row 2
    }
    
    // IMU tilt_y indicator: positive tilt = magenta dot moves UP (toward row 0)
    // Map tilt_y range (-45 to +45) to y position (5 to 0)
    float tilt_y_clamped = tilt_y;
    if (tilt_y_clamped < -45.0f) tilt_y_clamped = -45.0f;
    if (tilt_y_clamped > 45.0f) tilt_y_clamped = 45.0f;
    int y_pos = (int)((45.0f - tilt_y_clamped) * 5.0f / 90.0f);
    if (y_pos >= 0 && y_pos < matrix_height) {
        set_pixel(pixels, matrix_width / 2, y_pos, (struct led_rgb){60, 0, 60});  // Magenta at center column
    }
}

// Helper: HSV to RGB conversion with brightness capping
static struct led_rgb hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v) {
    struct led_rgb rgb = {0, 0, 0};
    
    // Cap brightness to 60/255 max (half of previous 120)
    if (v > 60) v = 60;
    
    if (s == 0) {
        rgb.r = rgb.g = rgb.b = v;
        return rgb;
    }
    
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;
    
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
        case 0: rgb.r = v; rgb.g = t; rgb.b = p; break;
        case 1: rgb.r = q; rgb.g = v; rgb.b = p; break;
        case 2: rgb.r = p; rgb.g = v; rgb.b = t; break;
        case 3: rgb.r = p; rgb.g = q; rgb.b = v; break;
        case 4: rgb.r = t; rgb.g = p; rgb.b = v; break;
        default: rgb.r = v; rgb.g = p; rgb.b = q; break;
    }
    
    return rgb;
}

// ============================================================================
// ANIMATION: Spectrum Bars
// ============================================================================
static void anim_spectrum_bars(struct led_rgb *pixels) {
    clear_pixels(pixels);
    
    // Draw 40 frequency bars, one per column (1:1 mapping)
    for (int x = 0; x < matrix_width; x++) {
        uint8_t energy = audio_viz_get_band(x);  // Each column = its own band
        
        // Calculate height: 0-254 maps to 0-5 pixels, 255 maps to full 6 pixels
        int height;
        if (energy == 255) {
            height = matrix_height;
        } else {
            height = (energy * matrix_height) / 255;
        }
        
        // Color gradient across all columns
        uint8_t hue = (x * 255) / matrix_width;
        struct led_rgb color = hsv_to_rgb(hue, 255, 40);  // Max 40 brightness
        
        // Draw bar from bottom up
        for (int y = 0; y < height && y < matrix_height; y++) {
            set_pixel(pixels, x, matrix_height - 1 - y, color);
        }
    }
}

// ============================================================================
// ANIMATION: VU Meter (40 columns with rainbow gradient and peak hold)
// ============================================================================
static void anim_vu_meter(struct led_rgb *pixels, uint32_t delta_time) {
    clear_pixels(pixels);
    
    // Draw 40 VU meters vertically, one per column with peak hold
    for (int x = 0; x < matrix_width; x++) {
        uint8_t energy = audio_viz_get_band(x);
        int height = (energy * matrix_height) / 255;
        
        // Update peak hold
        if (energy > peak_hold[x]) {
            peak_hold[x] = energy;
            peak_hold_time[x] = 0;
        } else {
            peak_hold_time[x] += delta_time;
            if (peak_hold_time[x] > 100) {  // Decay every 100ms
                peak_hold_time[x] = 0;
                if (peak_hold[x] > 5) {
                    peak_hold[x] -= 5;  // Drop by 5 units for faster decay
                } else {
                    peak_hold[x] = 0;
                }
            }
        }
        
        int peak_y = matrix_height - ((peak_hold[x] * matrix_height) / 255);
        
        // Rainbow color gradient across columns (same as spectrum bars)
        uint8_t hue = (x * 255) / matrix_width;
        struct led_rgb color = hsv_to_rgb(hue, 255, 40);  // Max 40 brightness
        
        // Draw bar from bottom up
        for (int y = matrix_height - height; y < matrix_height; y++) {
            set_pixel(pixels, x, y, color);
        }
        
        // Peak indicator (white dot)
        if (peak_y >= 0 && peak_y < matrix_height) {
            set_pixel(pixels, x, peak_y, (struct led_rgb){60, 60, 60});  // White peak dot
        }
    }
}

// ============================================================================
// ANIMATION: Pulse (bass-driven expanding circle)
// ============================================================================
static void anim_pulse(struct led_rgb *pixels) {
    clear_pixels(pixels);
    
    uint8_t bass = audio_viz_get_bass();
    
    // Aggressive remapping based on observed data: talking ~140-190, music ~200-253
    // Use 190 as floor, map 190-240 to 0-255 for better dynamic range
    int remapped_bass = 0;
    if (bass > 190) {
        remapped_bass = ((bass - 190) * 255) / 50;  // Map 190-240 to 0-255
        if (remapped_bass > 255) remapped_bass = 255;
    }
    
    float radius = (remapped_bass * 20.0f) / 255.0f;
    
    int cx = matrix_width / 2;
    int cy = matrix_height / 2;
    
    // Draw expanding circle
    for (int y = 0; y < matrix_height; y++) {
        for (int x = 0; x < matrix_width; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist < radius && dist > radius - 2.0f) {
                uint8_t brightness = remapped_bass;
                struct led_rgb color = hsv_to_rgb(160, 255, brightness); // Cyan
                set_pixel(pixels, x, y, color);
            }
        }
    }
    
    // Add center point for beat
    if (audio_viz_beat_detected()) {
        for (int y = cy - 1; y <= cy + 1; y++) {
            for (int x = cx - 1; x <= cx + 1; x++) {
                set_pixel(pixels, x, y, (struct led_rgb){200, 200, 200});
            }
        }
    }
}

// ============================================================================
// ANIMATION: Waveform
// ============================================================================
static void anim_waveform(struct led_rgb *pixels) {
    clear_pixels(pixels);
    
    // Add new volume sample to buffer
    waveform_buffer[waveform_idx] = audio_viz_get_volume();
    waveform_idx = (waveform_idx + 1) % WAVEFORM_HISTORY;
    
    // Draw waveform scrolling left
    for (int x = 0; x < matrix_width && x < WAVEFORM_HISTORY; x++) {
        int buf_idx = (waveform_idx + x) % WAVEFORM_HISTORY;
        uint8_t val = waveform_buffer[buf_idx];
        int y = (val * matrix_height) / 255;
        
        // Draw point with gradient
        uint8_t hue = (x * 255) / matrix_width;
        struct led_rgb color = hsv_to_rgb(hue, 255, 80);
        
        if (y < matrix_height) {
            set_pixel(pixels, matrix_width - 1 - x, matrix_height - 1 - y, color);
        }
    }
}

// ============================================================================
// ANIMATION: Gravity Particles
// ============================================================================
static void anim_gravity_particles(struct led_rgb *pixels, float tilt_x, float tilt_y, uint32_t delta_time) {
    clear_pixels(pixels);
    
    float dt = delta_time / 1000.0f;
    
    // Spawn new particles occasionally
    static uint32_t spawn_timer = 0;
    spawn_timer += delta_time;
    if (spawn_timer > 100) {
        spawn_timer = 0;
        
        // Find inactive particle
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life == 0) {
                particles[i].x = sys_rand32_get() % matrix_width;
                particles[i].y = 0;
                particles[i].vx = 0;
                particles[i].vy = 0;
                particles[i].life = 255;
                uint8_t hue = sys_rand32_get() % 255;
                particles[i].color = hsv_to_rgb(hue, 255, 40);  // Max 40 brightness
                break;
            }
        }
    }
    
    // Update and draw particles
    float gravity_x = tilt_x * 0.3f;
    float gravity_y = tilt_y * 0.3f;
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life == 0) continue;
        
        // Apply gravity based on tilt
        particles[i].vx += gravity_x * dt * 10.0f;
        particles[i].vy += gravity_y * dt * 10.0f;
        
        // Update position
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        
        // Bounce off edges
        if (particles[i].x < 0) {
            particles[i].x = 0;
            particles[i].vx = -particles[i].vx * 0.7f;
        }
        if (particles[i].x >= matrix_width) {
            particles[i].x = matrix_width - 1;
            particles[i].vx = -particles[i].vx * 0.7f;
        }
        if (particles[i].y < 0) {
            particles[i].y = 0;
            particles[i].vy = -particles[i].vy * 0.7f;
        }
        if (particles[i].y >= matrix_height) {
            particles[i].y = matrix_height - 1;
            particles[i].vy = -particles[i].vy * 0.7f;
        }
        
        // Friction
        particles[i].vx *= 0.98f;
        particles[i].vy *= 0.98f;
        
        // Draw particle
        int px = (int)particles[i].x;
        int py = (int)particles[i].y;
        set_pixel(pixels, px, py, particles[i].color);
    }
}

// ============================================================================
// ANIMATION: Tilt Gradient
// ============================================================================
static void anim_tilt_gradient(struct led_rgb *pixels, float tilt_x, float tilt_y) {
    clear_pixels(pixels);
    
    // Create color gradient based on tilt direction
    float angle = atan2f(tilt_y, tilt_x);
    uint8_t base_hue = (uint8_t)((angle + 3.14159f) * 127.65f / 3.14159f);
    
    for (int y = 0; y < matrix_height; y++) {
        for (int x = 0; x < matrix_width; x++) {
            // Distance from center
            float dx = (x - matrix_width / 2.0f) / matrix_width * 2.0f;
            float dy = (y - matrix_height / 2.0f) / matrix_height * 2.0f;
            
            // Angle to this pixel
            float pixel_angle = atan2f(dy, dx);
            float angle_diff = fabsf(pixel_angle - angle);
            if (angle_diff > 3.14159f) angle_diff = 6.28318f - angle_diff;
            
            uint8_t brightness = (uint8_t)(255 - (angle_diff * 80.0f));
            uint8_t hue = base_hue + (uint8_t)(angle_diff * 30.0f);
            
            struct led_rgb color = hsv_to_rgb(hue, 255, brightness / 3);
            set_pixel(pixels, x, y, color);
        }
    }
}

// ============================================================================
// ANIMATION: Shake Burst
// ============================================================================
static void anim_shake_burst(struct led_rgb *pixels, float tilt_x, float tilt_y, uint32_t delta_time) {
    // Detect shake (rapid tilt change)
    float delta_tilt = fabsf(tilt_x - prev_tilt_x) + fabsf(tilt_y - prev_tilt_y);
    prev_tilt_x = tilt_x;
    prev_tilt_y = tilt_y;
    
    if (delta_tilt > 30.0f) {
        shake_active = true;
        shake_time = 0;
        
        // Spawn particles in all directions
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].x = matrix_width / 2.0f;
            particles[i].y = matrix_height / 2.0f;
            float angle = (i * 6.28318f) / MAX_PARTICLES;
            float speed = 5.0f + (sys_rand32_get() % 50) / 10.0f;
            particles[i].vx = cosf(angle) * speed;
            particles[i].vy = sinf(angle) * speed;
            particles[i].life = 255;
            uint8_t hue = (i * 255) / MAX_PARTICLES;
            particles[i].color = hsv_to_rgb(hue, 255, 50);  // Max 50 brightness
        }
    }
    
    if (shake_active) {
        shake_time += delta_time;
        if (shake_time > 2000) {
            shake_active = false;
        }
    }
    
    clear_pixels(pixels);
    
    if (shake_active) {
        float dt = delta_time / 1000.0f;
        
        // Update particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            
            // Fade out
            if (particles[i].life > 5) {
                particles[i].life -= 5;
            }
            
            int px = (int)particles[i].x;
            int py = (int)particles[i].y;
            
            if (px >= 0 && px < matrix_width && py >= 0 && py < matrix_height) {
                struct led_rgb color = particles[i].color;
                color.r = (color.r * particles[i].life) / 255;
                color.g = (color.g * particles[i].life) / 255;
                color.b = (color.b * particles[i].life) / 255;
                set_pixel(pixels, px, py, color);
            }
        }
    }
}

// ============================================================================
// ANIMATION: Audio Rain
// ============================================================================
static void anim_audio_rain(struct led_rgb *pixels, uint32_t delta_time) {
    // Fade existing pixels
    for (int i = 0; i < matrix_width * matrix_height; i++) {
        if (pixels[i].r > 10) pixels[i].r -= 10;
        else pixels[i].r = 0;
        if (pixels[i].g > 10) pixels[i].g -= 10;
        else pixels[i].g = 0;
        if (pixels[i].b > 10) pixels[i].b -= 10;
        else pixels[i].b = 0;
    }
    
    // Spawn new rain drops based on audio
    uint8_t volume = audio_viz_get_volume();
    if (volume > 50 && (sys_rand32_get() % 3) == 0) {
        int x = sys_rand32_get() % matrix_width;
        uint8_t hue = (x * 255) / matrix_width;
        uint8_t brightness = (volume > 60) ? 60 : volume;  // Cap to 60
        struct led_rgb color = hsv_to_rgb(hue, 255, brightness);
        set_pixel(pixels, x, 0, color);
    }
}

// ============================================================================
// ANIMATION: Bass Ripple
// ============================================================================
static void anim_bass_ripple(struct led_rgb *pixels, uint32_t delta_time) {
    clear_pixels(pixels);
    
    float dt = delta_time / 1000.0f;
    
    // Spawn new ripple on beat
    if (audio_viz_beat_detected()) {
        for (int i = 0; i < MAX_RIPPLES; i++) {
            if (!ripples[i].active) {
                ripples[i].x = matrix_width / 2.0f;
                ripples[i].y = matrix_height / 2.0f;
                ripples[i].radius = 0;
                ripples[i].intensity = 255;
                ripples[i].active = true;
                break;
            }
        }
    }
    
    // Update and draw ripples
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (!ripples[i].active) continue;
        
        ripples[i].radius += 8.0f * dt;
        if (ripples[i].intensity > 5) {
            ripples[i].intensity -= 5;
        } else {
            ripples[i].active = false;
            continue;
        }
        
        // Draw ripple
        for (int y = 0; y < matrix_height; y++) {
            for (int x = 0; x < matrix_width; x++) {
                float dx = x - ripples[i].x;
                float dy = y - ripples[i].y;
                float dist = sqrtf(dx * dx + dy * dy);
                
                if (fabsf(dist - ripples[i].radius) < 1.5f) {
                    uint8_t hue = (uint8_t)(ripples[i].radius * 10);
                    struct led_rgb color = hsv_to_rgb(hue, 255, ripples[i].intensity);
                    set_pixel(pixels, x, y, color);
                }
            }
        }
    }
}

// ============================================================================
// ANIMATION: Reactive Spiral (audio + tilt)
// ============================================================================
static void anim_reactive_spiral(struct led_rgb *pixels, float tilt_x, float tilt_y) {
    clear_pixels(pixels);
    
    static float spiral_angle = 0;
    spiral_angle += 0.1f;
    
    uint8_t bass = audio_viz_get_bass();
    float spiral_tightness = 0.3f + (bass / 255.0f) * 0.5f;
    
    // Center point affected by tilt
    float cx = (matrix_width / 2.0f) + (tilt_x / 90.0f) * 10.0f;
    float cy = (matrix_height / 2.0f) + (tilt_y / 90.0f) * 3.0f;
    
    // Draw spiral
    for (float t = 0; t < 20.0f; t += 0.2f) {
        float angle = t * spiral_tightness + spiral_angle;
        float radius = t;
        
        int x = (int)(cx + cosf(angle) * radius);
        int y = (int)(cy + sinf(angle) * radius / 1.5f);
        
        uint8_t hue = (uint8_t)(t * 12 + bass);
        struct led_rgb color = hsv_to_rgb(hue, 255, 80);
        set_pixel(pixels, x, y, color);
    }
}

// ============================================================================
// Public API
// ============================================================================

void animations_init(int width, int height) {
    matrix_width = width;
    matrix_height = height;
    
    // Initialize particle system
    memset(particles, 0, sizeof(particles));
    memset(ripples, 0, sizeof(ripples));
    memset(peak_hold, 0, sizeof(peak_hold));
    memset(peak_hold_time, 0, sizeof(peak_hold_time));
    memset(waveform_buffer, 0, sizeof(waveform_buffer));
    
    animations_ready = true;
}

void animations_update(struct led_rgb *pixels, float tilt_x, float tilt_y, uint32_t delta_time) {
    // Safety check - don't run if not initialized
    if (!animations_ready) {
        clear_pixels(pixels);
        return;
    }
    
    switch (current_mode) {
        case ANIM_TEST_MODE:
            anim_test_mode(pixels, tilt_x, tilt_y);
            break;
        case ANIM_VU_METER:
            anim_vu_meter(pixels, delta_time);
            break;
        case ANIM_SPECTRUM_BARS:
            anim_spectrum_bars(pixels);
            break;
        case ANIM_PULSE:
            anim_pulse(pixels);
            break;
        case ANIM_WAVEFORM:
            anim_waveform(pixels);
            break;
        case ANIM_GRAVITY_PARTICLES:
            anim_gravity_particles(pixels, tilt_x, tilt_y, delta_time);
            break;
        case ANIM_TILT_GRADIENT:
            anim_tilt_gradient(pixels, tilt_x, tilt_y);
            break;
        case ANIM_SHAKE_BURST:
            anim_shake_burst(pixels, tilt_x, tilt_y, delta_time);
            break;
        case ANIM_AUDIO_RAIN:
            anim_audio_rain(pixels, delta_time);
            break;
        case ANIM_BASS_RIPPLE:
            anim_bass_ripple(pixels, delta_time);
            break;
        case ANIM_REACTIVE_SPIRAL:
            anim_reactive_spiral(pixels, tilt_x, tilt_y);
            break;
        default:
            clear_pixels(pixels);
            break;
    }
}

void animations_set_mode(animation_mode_t mode) {
    if (mode < ANIM_MODE_COUNT) {
        current_mode = mode;
        
        // Reset state for new mode
        memset(particles, 0, sizeof(particles));
        memset(ripples, 0, sizeof(ripples));
        shake_active = false;
    }
}

animation_mode_t animations_get_mode(void) {
    return current_mode;
}

void animations_next_mode(void) {
    current_mode = (current_mode + 1) % ANIM_MODE_COUNT;
    animations_set_mode(current_mode);
}

const char* animations_get_mode_name(animation_mode_t mode) {
    const char* names[] = {
        "Test Mode",
        "VU Meter",
        "Spectrum Bars",
        "Pulse",
        "Waveform",
        "Gravity",
        "Tilt Gradient",
        "Shake Burst",
        "Audio Rain",
        "Bass Ripple",
        "Spiral"
    };
    
    if (mode < ANIM_MODE_COUNT) {
        return names[mode];
    }
    return "Unknown";
}
