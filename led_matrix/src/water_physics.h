/*
 * Copyright (c) 2025 Water Physics Module
 * Inspired by The Powder Toy's liquid simulation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef WATER_PHYSICS_H
#define WATER_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the water physics simulation
 * @param width Matrix width in pixels
 * @param height Matrix height in pixels
 * @param initial_fill_rows Number of rows to fill from bottom at start (0 for text mode)
 * @return 0 on success, negative error code on failure
 */
int water_physics_init(int width, int height, int initial_fill_rows);

/**
 * @brief Initialize water physics with text pattern
 * @param width Matrix width in pixels
 * @param height Matrix height in pixels
 * @param text Text to display in water
 * @return 0 on success, negative error code on failure
 */
int water_physics_init_text(int width, int height, const char *text);

/**
 * @brief Update tilt angles for the water physics simulation
 * @param tilt_x Roll angle in degrees (left/right tilt)
 * @param tilt_y Pitch angle in degrees (forward/backward tilt)
 */
void water_physics_set_tilt(float tilt_x, float tilt_y);

/**
 * @brief Update the water physics simulation by one step
 * 
 * Implements Powder-Toy style liquid physics:
 * - Velocity-based particle movement
 * - Gravity influences velocity
 * - Horizontal spreading when particles can't fall
 * - Stagnation detection for performance
 * - Collision dampening
 */
void water_physics_update(void);

/**
 * @brief Get the state of a specific cell in the water grid
 * @param x X coordinate
 * @param y Y coordinate
 * @return 1 if cell contains water, 0 if empty
 */
uint8_t water_physics_get_cell(int x, int y);

/**
 * @brief Reset the water simulation to initial state
 */
void water_physics_reset(void);

/**
 * @brief Clean up and deallocate water physics resources
 */
void water_physics_deinit(void);

#endif /* WATER_PHYSICS_H */
