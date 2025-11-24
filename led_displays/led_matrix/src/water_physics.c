/*
 * Copyright (c) 2025 Water Physics Module
 * Inspired by The Powder Toy's liquid simulation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "water_physics.h"
#include "font_5x5.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Particle structure (inspired by Powder Toy)
typedef struct {
	float vx;        // Velocity X
	float vy;        // Velocity Y
	uint8_t flags;   // Flags for stagnation, etc.
} water_particle_t;

// Flags
#define FLAG_STAGNANT   0x01  // Particle hasn't moved recently
#define FLAG_ACTIVE     0x02  // Particle moved this frame

// Water simulation state
static uint8_t *grid = NULL;              // 1 = water, 0 = empty
static water_particle_t *particles = NULL; // Particle properties
static int matrix_width = 0;
static int matrix_height = 0;
static int initial_fill = 0;

// Physics constants (inspired by Powder Toy WATR element)
#define GRAVITY         0.5f    // Gravity constant (scaled for small grid)
#define COLLISION_LOSS  0.75f   // Velocity loss on collision
#define VELOCITY_LOSS   0.95f   // Velocity dampening per frame
#define MAX_VELOCITY    4.0f    // Maximum velocity
#define SPREAD_RANGE    12      // How far to look for spreading (like Powder Toy's rt)

// Tilt state for gravity direction
static float tilt_x = 0.0f;  // Roll (left/right)
static float tilt_y = 0.0f;  // Pitch (forward/backward)
static float grav_x = 0.0f;  // Gravity vector X
static float grav_y = 1.0f;  // Gravity vector Y (default down)

/**
 * @brief Initialize the water physics simulation
 */
int water_physics_init(int width, int height, int initial_fill_rows)
{
	if (width <= 0 || height <= 0) {
		return -1;
	}

	matrix_width = width;
	matrix_height = height;
	initial_fill = initial_fill_rows;

	// Allocate grids
	grid = (uint8_t *)malloc(width * height * sizeof(uint8_t));
	particles = (water_particle_t *)malloc(width * height * sizeof(water_particle_t));

	if (!grid || !particles) {
		if (grid) free(grid);
		if (particles) free(particles);
		return -1;
	}

	// Initialize with water at bottom
	water_physics_reset();

	return 0;
}

/**
 * @brief Update tilt angles and calculate gravity vector
 */
void water_physics_set_tilt(float new_tilt_x, float new_tilt_y)
{
	tilt_x = new_tilt_x;
	tilt_y = new_tilt_y;

	// Convert tilt angles to proper gravity vector
	// tilt_x = roll (rotation around Y axis) - affects which edge is "down"
	// tilt_y = pitch (rotation around X axis) - affects forward/back tilt
	
	// Use sin/cos to properly convert angle to gravity components
	float angle_rad_x = tilt_x * 3.14159f / 180.0f;
	float angle_rad_y = tilt_y * 3.14159f / 180.0f;
	
	// Calculate gravity vector based on orientation
	// X component: left/right gravity (from roll)
	// Y component: up/down gravity (from roll, inverted when upside down)
	grav_x = sinf(angle_rad_x) * GRAVITY;
	grav_y = cosf(angle_rad_x) * GRAVITY;
	
	// Add forward/back tilt influence on X axis
	grav_x -= sinf(angle_rad_y) * GRAVITY * 0.5f;
}

/**
 * @brief Initialize water physics with text pattern
 */
int water_physics_init_text(int width, int height, const char *text)
{
	if (width <= 0 || height <= 0) {
		return -1;
	}

	matrix_width = width;
	matrix_height = height;
	initial_fill = 0;  // No bottom fill for text mode

	// Allocate grids
	grid = (uint8_t *)malloc(width * height * sizeof(uint8_t));
	particles = (water_particle_t *)malloc(width * height * sizeof(water_particle_t));

	if (!grid || !particles) {
		if (grid) free(grid);
		if (particles) free(particles);
		return -1;
	}

	memset(grid, 0, matrix_width * matrix_height);
	memset(particles, 0, matrix_width * matrix_height * sizeof(water_particle_t));

	// Calculate text dimensions
	int text_len = 0;
	while (text[text_len]) text_len++;
	// Align text to start at x=0 for proper segment alignment
	int start_x = 0;
	int start_y = (height - 5) / 2;  // Center vertically

	// Draw each character (8 pixels wide including spacing for segment alignment)
	for (int i = 0; i < text_len; i++) {
		char c = text[i];
		if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';  // Convert to uppercase
		if (c < ' ' || c > 'Z') c = ' ';  // Default to space
		
		int char_idx = c - ' ';
		const uint8_t *glyph = font_5x5[char_idx];
		
		int char_x = start_x + i * 8;  // 8 pixels per character for segment alignment
		
		// Draw 5x5 character
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < 5; col++) {
				if (glyph[row] & (1 << (4 - col))) {
					int px = char_x + col;
					int py = start_y + row;
					if (px >= 0 && px < width && py >= 0 && py < height) {
						int idx = py * width + px;
						grid[idx] = 1;
					}
				}
			}
		}
	}

	return 0;
}

/**
 * @brief Get the state of a specific cell in the water grid
 */
uint8_t water_physics_get_cell(int x, int y)
{
	if (x < 0 || x >= matrix_width || y < 0 || y >= matrix_height) {
		return 0;
	}
	return grid[y * matrix_width + x];
}

/**
 * @brief Reset the water simulation to initial state
 */
void water_physics_reset(void)
{
	if (!grid || !particles) {
		return;
	}

	memset(grid, 0, matrix_width * matrix_height);
	memset(particles, 0, matrix_width * matrix_height * sizeof(water_particle_t));

	// Fill bottom rows
	int start_row = matrix_height - initial_fill;
	if (start_row < 0) start_row = 0;

	for (int y = start_row; y < matrix_height; y++) {
		for (int x = 0; x < matrix_width; x++) {
			int idx = y * matrix_width + x;
			grid[idx] = 1;
			particles[idx].vx = 0.0f;
			particles[idx].vy = 0.0f;
			particles[idx].flags = 0;
		}
	}
}

/**
 * @brief Try to move a particle to a new position
 * @return 1 if moved, 0 if blocked
 */
static int try_move(int old_x, int old_y, int new_x, int new_y)
{
	// Bounds check
	if (new_x < 0 || new_x >= matrix_width || new_y < 0 || new_y >= matrix_height) {
		return 0;
	}

	int old_idx = old_y * matrix_width + old_x;
	int new_idx = new_y * matrix_width + new_x;

	// Check if destination is empty
	if (grid[new_idx] == 0) {
		// Move particle
		grid[new_idx] = 1;
		grid[old_idx] = 0;
		particles[new_idx] = particles[old_idx];
		particles[new_idx].flags |= FLAG_ACTIVE;
		memset(&particles[old_idx], 0, sizeof(water_particle_t));
		return 1;
	}

	return 0;
}

/**
 * @brief Clamp velocity to maximum
 */
static inline float clamp_velocity(float v)
{
	if (v > MAX_VELOCITY) return MAX_VELOCITY;
	if (v < -MAX_VELOCITY) return -MAX_VELOCITY;
	return v;
}

/**
 * @brief Update liquid - Powder Toy style with velocity and spreading
 */
void water_physics_update(void)
{
	if (!grid || !particles) {
		return;
	}

	// Clear active flags from previous frame
	for (int i = 0; i < matrix_width * matrix_height; i++) {
		particles[i].flags &= ~FLAG_ACTIVE;
	}

	// Process particles in random-ish order to avoid directional bias
	// Alternate scan direction each frame for better mixing
	static int scan_dir = 1;
	scan_dir = -scan_dir;

	int y_start = (scan_dir > 0) ? 0 : matrix_height - 1;
	int y_end = (scan_dir > 0) ? matrix_height : -1;

	for (int y = y_start; y != y_end; y += scan_dir) {
		for (int x = 0; x < matrix_width; x++) {
			int idx = y * matrix_width + x;
			
			if (grid[idx] == 0) continue;  // Empty cell
			if (particles[idx].flags & FLAG_ACTIVE) continue;  // Already moved this frame

			water_particle_t *p = &particles[idx];
			
			// Apply gravity
			p->vx += grav_x;
			p->vy += grav_y;

			// Clamp velocities
			p->vx = clamp_velocity(p->vx);
			p->vy = clamp_velocity(p->vy);

			// Apply velocity dampening AFTER gravity
			p->vx *= VELOCITY_LOSS;
			p->vy *= VELOCITY_LOSS;

			// Calculate target position (favor movement even with small velocities)
			int new_x = x;
			int new_y = y;
			
			if (fabsf(p->vx) > 0.3f) new_x = x + (int)roundf(p->vx);
			if (fabsf(p->vy) > 0.3f) new_y = y + (int)roundf(p->vy);

			// Clamp to bounds
			if (new_x < 0) new_x = 0;
			if (new_x >= matrix_width) new_x = matrix_width - 1;
			if (new_y < 0) new_y = 0;
			if (new_y >= matrix_height) new_y = matrix_height - 1;

			// Try direct movement
			if (try_move(x, y, new_x, new_y)) {
				p->flags &= ~FLAG_STAGNANT;
				continue;
			}

			// If blocked, try falling straight down
			if (new_x == x && new_y == y) {
				// No movement calculated, try direct fall
				if (grav_y > 0.1f && try_move(x, y, x, y + 1)) {
					p->flags &= ~FLAG_STAGNANT;
					continue;
				}
			}

			// If still blocked, try spreading horizontally (Powder Toy style)
			// Only spread if we were trying to move down
			if (fabsf(grav_y) > 0.5f || fabsf(p->vy) > 0.5f) {
				bool stagnant = (p->flags & FLAG_STAGNANT);
				int range = stagnant ? 3 : SPREAD_RANGE;  // Reduce range for stagnant particles
				
				// Try spreading left and right
				int dir = (x & 1) ? 1 : -1;  // Alternate direction based on position
				bool moved = false;

				for (int spread = 1; spread <= range && !moved; spread++) {
					// Try in alternating direction first
					int try_x = x + (spread * dir);
					if (try_x >= 0 && try_x < matrix_width) {
						// Check if this horizontal position is open and has support below
						int try_idx = y * matrix_width + try_x;
						if (grid[try_idx] == 0) {
							// Check if there's water below to rest on
							int below_y = y + (int)roundf(grav_y);
							if (below_y < 0 || below_y >= matrix_height || 
							    grid[below_y * matrix_width + try_x] != 0) {
								if (try_move(x, y, try_x, y)) {
									p->vx = (float)dir * 0.5f;  // Give it sideways velocity
									p->vy *= COLLISION_LOSS;
									p->flags &= ~FLAG_STAGNANT;
									moved = true;
									break;
								}
							}
						}
					}

					// Try opposite direction
					try_x = x - (spread * dir);
					if (try_x >= 0 && try_x < matrix_width) {
						int try_idx = y * matrix_width + try_x;
						if (grid[try_idx] == 0) {
							int below_y = y + (int)roundf(grav_y);
							if (below_y < 0 || below_y >= matrix_height || 
							    grid[below_y * matrix_width + try_x] != 0) {
								if (try_move(x, y, try_x, y)) {
									p->vx = (float)(-dir) * 0.5f;
									p->vy *= COLLISION_LOSS;
									p->flags &= ~FLAG_STAGNANT;
									moved = true;
									break;
								}
							}
						}
					}
				}

				if (!moved) {
					// Particle couldn't move at all - mark as stagnant
					p->flags |= FLAG_STAGNANT;
					p->vx *= 0.5f;  // Slow down stagnant particles
					p->vy *= 0.5f;
				}
			} else {
				// Moving mostly horizontally, dampen and mark stagnant
				p->vx *= COLLISION_LOSS;
				p->vy *= COLLISION_LOSS;
				p->flags |= FLAG_STAGNANT;
			}
		}
	}
}

/**
 * @brief Clean up and deallocate water physics resources
 */
void water_physics_deinit(void)
{
	if (grid) {
		free(grid);
		grid = NULL;
	}
	if (particles) {
		free(particles);
		particles = NULL;
	}
	matrix_width = 0;
	matrix_height = 0;
}
