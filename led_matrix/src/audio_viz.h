/*
 * Audio Visualizer Module for LED Matrix
 * Captures audio from DMIC, performs FFT, and provides frequency band data
 */

#ifndef AUDIO_VIZ_H
#define AUDIO_VIZ_H

#include <stdint.h>
#include <stdbool.h>

// Audio configuration
#define AUDIO_SAMPLE_RATE      16000
#define AUDIO_FFT_SIZE         512
#define AUDIO_MEL_BANDS        40  // 40 bands for 40-column display (1 band per column)
#define AUDIO_LOW_BANDS        13  // First 13 bands = bass
#define AUDIO_MID_BANDS        14  // Middle 14 bands = mids
#define AUDIO_HIGH_BANDS       13  // Last 13 bands = highs

/**
 * @brief Initialize audio visualization system
 * @return 0 on success, negative error code on failure
 */
int audio_viz_init(void);

/**
 * @brief Start audio capture and processing
 * @return 0 on success, negative error code on failure
 */
int audio_viz_start(void);

/**
 * @brief Stop audio capture
 */
void audio_viz_stop(void);

/**
 * @brief Check if audio system is available
 * @return true if audio is working, false otherwise
 */
bool audio_viz_available(void);

/**
 * @brief Process next audio buffer (call periodically)
 * Non-blocking - returns immediately if no data available
 */
void audio_viz_process(void);

/**
 * @brief Get normalized band energy (0-255)
 * @param band Band index (0 to AUDIO_MEL_BANDS-1)
 * @return Energy level 0-255
 */
uint8_t audio_viz_get_band(int band);

/**
 * @brief Get bass energy (average of low bands)
 * @return Bass level 0-255
 */
uint8_t audio_viz_get_bass(void);

/**
 * @brief Get mid energy (average of mid bands)
 * @return Mid level 0-255
 */
uint8_t audio_viz_get_mids(void);

/**
 * @brief Get high energy (average of high bands)
 * @return High level 0-255
 */
uint8_t audio_viz_get_highs(void);

/**
 * @brief Get overall volume level
 * @return Volume level 0-255
 */
uint8_t audio_viz_get_volume(void);

/**
 * @brief Detect beat (sudden bass increase)
 * @return true if beat detected this frame
 */
bool audio_viz_beat_detected(void);

#endif // AUDIO_VIZ_H
