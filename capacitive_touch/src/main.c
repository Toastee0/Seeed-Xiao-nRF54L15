/*
 * Capacitive Touch Sample using COMP Peripheral
 * 
 * This sample demonstrates capacitive touch sensing using the nRF54L15 COMP
 * peripheral on pins D0-D3 (P1.04-P1.07 / AIN0-AIN3).
 * 
 * The comparator is configured in single-ended mode for capacitive sensing.
 * Touch detection is based on voltage changes when a finger approaches/touches
 * the sensor pad.
 * 
 * Hardware connections:
 * - D0 (P1.04): Touch sensor pad 0
 * - D1 (P1.05): Touch sensor pad 1  
 * - D2 (P1.06): Touch sensor pad 2
 * - D3 (P1.07): Touch sensor pad 3
 * - Built-in LED: Visual feedback for touches
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <hal/nrf_comp.h>
#include <nrfx_comp.h>

LOG_MODULE_REGISTER(capacitive_touch, LOG_LEVEL_INF);

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define TOUCH_CHANNELS 4
#define TOUCH_THRESHOLD_MV 100    // Threshold in millivolts for touch detection
#define SAMPLE_INTERVAL_MS 50     // How often to sample touch sensors
#define TOUCH_DEBOUNCE_MS 100     // Debounce time for touch detection

// Capacitive touch configuration
#define COMP_HYST_50MV 2          // Hysteresis setting (50mV)
#define COMP_REF_VDD_DIV2 4       // Reference: VDD/2
#define COMP_SPEED_NORMAL 0       // Normal speed mode

// ============================================================================
// HARDWARE DEFINITIONS
// ============================================================================

// Built-in LED for visual feedback
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// Touch sensor GPIO pins (for configuration)
static const struct gpio_dt_spec touch_pins[TOUCH_CHANNELS] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(touch0), gpios),  // D0 (P1.04)
    GPIO_DT_SPEC_GET(DT_ALIAS(touch1), gpios),  // D1 (P1.05)  
    GPIO_DT_SPEC_GET(DT_ALIAS(touch2), gpios),  // D2 (P1.06)
    GPIO_DT_SPEC_GET(DT_ALIAS(touch3), gpios),  // D3 (P1.07)
};

// ADC channels for touch sensing  
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// COMP device handle
static const struct device *comp_dev;
static nrfx_comp_t comp_instance = NRFX_COMP_INSTANCE(0);

// Touch state tracking
static bool touch_state[TOUCH_CHANNELS] = {false};
static bool prev_touch_state[TOUCH_CHANNELS] = {false};
static int64_t last_touch_time[TOUCH_CHANNELS] = {0};

// Baseline values for each channel (calibrated at startup)
static uint16_t baseline_values[TOUCH_CHANNELS] = {0};

// Current ADC readings
static uint16_t current_readings[TOUCH_CHANNELS] = {0};

// ============================================================================
// COMP PERIPHERAL FUNCTIONS
// ============================================================================

/**
 * @brief COMP event handler callback
 * 
 * @param event COMP event type
 * @param context User context (channel number)
 */
static void comp_event_handler(nrfx_comp_evt_t const *event, void *context)
{
    uint32_t channel = (uint32_t)context;
    
    switch (event->type) {
        case NRFX_COMP_EVT_READY:
            LOG_DBG("COMP ready on channel %d", channel);
            break;
            
        case NRFX_COMP_EVT_DOWN:
            LOG_DBG("COMP down event on channel %d", channel);
            break;
            
        case NRFX_COMP_EVT_UP:
            LOG_DBG("COMP up event on channel %d", channel);
            break;
            
        case NRFX_COMP_EVT_CROSS:
            LOG_DBG("COMP cross event on channel %d", channel);
            break;
    }
}

/**
 * @brief Initialize COMP peripheral for capacitive touch
 * 
 * @return 0 on success, negative error code otherwise
 */
static int comp_init(void)
{
    nrfx_err_t err;
    
    // Get COMP device
    comp_dev = DEVICE_DT_GET(DT_NODELABEL(comp));
    if (!device_is_ready(comp_dev)) {
        LOG_ERR("COMP device not ready");
        return -ENODEV;
    }
    
    // Configure COMP for capacitive sensing
    nrfx_comp_config_t config = NRFX_COMP_DEFAULT_CONFIG;
    config.reference = NRF_COMP_REF_VDD;      // Use VDD as reference
    config.main_mode = NRF_COMP_MAIN_MODE_SE; // Single-ended mode
    config.threshold = {                      // Threshold configuration
        .th_down = 32,                        // Lower threshold (VDD * 32/63)
        .th_up = 33                           // Upper threshold (VDD * 33/63)
    };
    config.speed_mode = NRF_COMP_SP_MODE_Normal; // Normal speed
    config.hyst = NRF_COMP_HYST_50mV;         // 50mV hysteresis
    config.interrupt_priority = NRFX_COMP_DEFAULT_CONFIG_IRQ_PRIORITY;
    
    // Initialize COMP
    err = nrfx_comp_init(&comp_instance, &config, comp_event_handler);
    if (err != NRFX_SUCCESS) {
        LOG_ERR("Failed to initialize COMP: %d", err);
        return -EIO;
    }
    
    LOG_INF("COMP peripheral initialized for capacitive touch");
    return 0;
}

/**
 * @brief Configure COMP to read from specific analog input
 * 
 * @param ain_channel Analog input channel (0-3 for AIN0-AIN3)
 * @return 0 on success, negative error code otherwise
 */
static int comp_select_channel(uint8_t ain_channel)
{
    if (ain_channel >= TOUCH_CHANNELS) {
        return -EINVAL;
    }
    
    // Stop COMP if running
    nrfx_comp_stop();
    
    // Configure pin select for the desired AIN channel
    nrf_comp_input_select(NRF_COMP, (nrf_comp_input_t)ain_channel);
    
    return 0;
}

/**
 * @brief Start COMP measurement on selected channel
 * 
 * @return 0 on success, negative error code otherwise
 */
static int comp_start_measurement(void)
{
    nrfx_comp_start(0, NULL);  // Start with no context
    return 0;
}

/**
 * @brief Get COMP result
 * 
 * @return true if VIN+ > VIN-, false otherwise
 */
static bool comp_get_result(void)
{
    return nrf_comp_result_get(NRF_COMP) == NRF_COMP_RESULT_ABOVE;
}

// ============================================================================
// ADC HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Read ADC value from specified channel
 * 
 * @param channel Channel index (0-3)
 * @param value Pointer to store the ADC reading
 * @return 0 on success, negative error code otherwise
 */
static int read_adc_channel(uint8_t channel, uint16_t *value)
{
    if (channel >= TOUCH_CHANNELS) {
        return -EINVAL;
    }
    
    struct adc_sequence sequence = {
        .buffer = value,
        .buffer_size = sizeof(*value),
        .resolution = adc_channels[channel].resolution,
    };
    
    int ret = adc_read_dt(&adc_channels[channel], &sequence);
    if (ret < 0) {
        LOG_ERR("Failed to read ADC channel %d: %d", channel, ret);
        return ret;
    }
    
    return 0;
}

// ============================================================================
// CAPACITIVE TOUCH FUNCTIONS
// ============================================================================

/**
 * @brief Calibrate baseline values for all touch channels
 * 
 * This function should be called at startup when no fingers are touching
 * the sensor pads to establish baseline reference values.
 * 
 * @return 0 on success, negative error code otherwise
 */
static int calibrate_touch_baselines(void)
{
    LOG_INF("Calibrating touch sensor baselines...");
    
    // Take multiple samples and average them for each channel
    const int samples = 10;
    
    for (int ch = 0; ch < TOUCH_CHANNELS; ch++) {
        uint32_t sum = 0;
        
        for (int i = 0; i < samples; i++) {
            uint16_t reading;
            int ret = read_adc_channel(ch, &reading);
            if (ret < 0) {
                return ret;
            }
            sum += reading;
            k_msleep(10);  // Small delay between samples
        }
        
        baseline_values[ch] = sum / samples;
        LOG_INF("Channel %d baseline: %d", ch, baseline_values[ch]);
    }
    
    LOG_INF("Touch sensor calibration complete");
    return 0;
}

/**
 * @brief Detect touch on specific channel using COMP
 * 
 * @param channel Channel to check (0-3)
 * @return true if touch detected, false otherwise
 */
static bool detect_touch_comp(uint8_t channel)
{
    if (channel >= TOUCH_CHANNELS) {
        return false;
    }
    
    // Configure COMP for this channel
    if (comp_select_channel(channel) < 0) {
        return false;
    }
    
    // Start measurement
    comp_start_measurement();
    
    // Give COMP time to settle (typically ~10us for nRF54L15)
    k_busy_wait(20);
    
    // Get result
    bool result = comp_get_result();
    
    // Stop COMP
    nrfx_comp_stop();
    
    return result;
}

/**
 * @brief Detect touch using ADC-based method as fallback
 * 
 * @param channel Channel to check (0-3)
 * @return true if touch detected, false otherwise
 */
static bool detect_touch_adc(uint8_t channel)
{
    if (channel >= TOUCH_CHANNELS) {
        return false;
    }
    
    uint16_t reading;
    if (read_adc_channel(channel, &reading) < 0) {
        return false;
    }
    
    current_readings[channel] = reading;
    
    // Touch detected if reading differs significantly from baseline
    int16_t delta = (int16_t)reading - (int16_t)baseline_values[channel];
    return (abs(delta) > (TOUCH_THRESHOLD_MV * 4095 / 3300)); // Convert mV to ADC counts
}

/**
 * @brief Process touch detection for all channels
 */
static void process_touch_detection(void)
{
    int64_t current_time = k_uptime_get();
    bool any_touch = false;
    
    for (int ch = 0; ch < TOUCH_CHANNELS; ch++) {
        // Try COMP-based detection first, fall back to ADC
        bool touch_detected = detect_touch_comp(ch);
        if (!touch_detected) {
            touch_detected = detect_touch_adc(ch);
        }
        
        // Debounce touch detection
        if (touch_detected != prev_touch_state[ch]) {
            if ((current_time - last_touch_time[ch]) > TOUCH_DEBOUNCE_MS) {
                touch_state[ch] = touch_detected;
                last_touch_time[ch] = current_time;
                
                if (touch_detected) {
                    LOG_INF("Touch detected on channel %d", ch);
                    any_touch = true;
                } else {
                    LOG_INF("Touch released on channel %d", ch);
                }
            }
        } else {
            touch_state[ch] = touch_detected;
        }
        
        prev_touch_state[ch] = touch_detected;
        
        if (touch_state[ch]) {
            any_touch = true;
        }
    }
    
    // Control LED based on any touch detection
    gpio_pin_set_dt(&led, any_touch ? 1 : 0);
}

/**
 * @brief Print current touch status
 */
static void print_touch_status(void)
{
    static int64_t last_print = 0;
    int64_t current_time = k_uptime_get();
    
    // Print status every 2 seconds
    if ((current_time - last_print) > 2000) {
        LOG_INF("Touch Status - D0:%s D1:%s D2:%s D3:%s",
               touch_state[0] ? "ON " : "OFF",
               touch_state[1] ? "ON " : "OFF", 
               touch_state[2] ? "ON " : "OFF",
               touch_state[3] ? "ON " : "OFF");
               
        LOG_INF("ADC Readings - D0:%d D1:%d D2:%d D3:%d",
               current_readings[0], current_readings[1],
               current_readings[2], current_readings[3]);
               
        last_print = current_time;
    }
}

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main(void)
{
    int ret;
    
    LOG_INF("Starting Capacitive Touch Sample");
    LOG_INF("nRF54L15 COMP-based touch sensing on D0-D3");
    
    // ========================================================================
    // Initialize LED
    // ========================================================================
    
    if (!gpio_is_ready_dt(&led)) {
        LOG_ERR("LED GPIO device not ready");
        return -ENODEV;
    }
    
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure LED pin: %d", ret);
        return ret;
    }
    
    LOG_INF("LED initialized on P2.0");
    
    // ========================================================================
    // Initialize Touch Sensor GPIO Pins
    // ========================================================================
    
    for (int i = 0; i < TOUCH_CHANNELS; i++) {
        if (!gpio_is_ready_dt(&touch_pins[i])) {
            LOG_ERR("Touch pin %d GPIO device not ready", i);
            return -ENODEV;
        }
        
        // Configure as analog input (high impedance)
        ret = gpio_pin_configure_dt(&touch_pins[i], GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0) {
            LOG_ERR("Failed to configure touch pin %d: %d", i, ret);
            return ret;
        }
    }
    
    LOG_INF("Touch sensor pins initialized (D0-D3)");
    
    // ========================================================================
    // Initialize ADC
    // ========================================================================
    
    for (int i = 0; i < TOUCH_CHANNELS; i++) {
        if (!adc_is_ready_dt(&adc_channels[i])) {
            LOG_ERR("ADC channel %d not ready", i);
            return -ENODEV;
        }
        
        ret = adc_channel_setup_dt(&adc_channels[i]);
        if (ret < 0) {
            LOG_ERR("Failed to setup ADC channel %d: %d", i, ret);
            return ret;
        }
    }
    
    LOG_INF("ADC channels initialized");
    
    // ========================================================================
    // Initialize COMP Peripheral
    // ========================================================================
    
    ret = comp_init();
    if (ret < 0) {
        LOG_ERR("Failed to initialize COMP peripheral");
        return ret;
    }
    
    // ========================================================================
    // Calibrate Touch Baselines
    // ========================================================================
    
    LOG_INF("Please ensure no fingers are touching sensor pads...");
    k_msleep(3000);  // Give user time to remove fingers
    
    ret = calibrate_touch_baselines();
    if (ret < 0) {
        LOG_ERR("Failed to calibrate touch baselines");
        return ret;
    }
    
    // ========================================================================
    // Main Touch Detection Loop
    // ========================================================================
    
    LOG_INF("Touch detection active - touch D0, D1, D2, or D3 pads");
    LOG_INF("LED will light up when any touch is detected");
    
    while (1) {
        // Process touch detection for all channels
        process_touch_detection();
        
        // Print periodic status
        print_touch_status();
        
        // Sleep until next sample interval
        k_msleep(SAMPLE_INTERVAL_MS);
    }
    
    return 0;
}