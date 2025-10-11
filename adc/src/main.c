/*
 * ADC Variable Blink Sample
 * 
 * This sample demonstrates:
 * - Reading analog input from a potentiometer using ADC (Analog-to-Digital Converter)
 * - Converting ADC raw values to millivolts
 * - Controlling LED blink rate based on analog voltage
 * 
 * Hardware connections:
 * - Potentiometer connected to A0 (P1.04)
 * - Built-in LED on P2.0 will blink
 * 
 * Behavior:
 * - Turn pot LEFT (0V)    -> LED blinks SLOWLY (3.3 second intervals)
 * - Turn pot RIGHT (3.3V) -> LED blinks FAST (near-continuous)
 */

// Include Zephyr RTOS kernel functions (delays, threads, etc.)
#include <zephyr/kernel.h>

// Include ADC (Analog-to-Digital Converter) driver
#include <zephyr/drivers/adc.h>

// Include GPIO (General Purpose Input/Output) driver for LED control
#include <zephyr/drivers/gpio.h>

// Include logging system for debug output
#include <zephyr/logging/log.h>

// Register this module with the logging system
// Messages will appear with "adc_blink_example" prefix
LOG_MODULE_REGISTER(adc_blink_example, CONFIG_LOG_DEFAULT_LEVEL);

// ============================================================================
// ADC CONFIGURATION
// ============================================================================

// Compile-time check: Make sure the devicetree overlay file defines ADC channels
// If this fails, you forgot to include the board-specific overlay file
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified for ADC channels"
#endif

// Macro helper: Gets ADC channel specifications from devicetree
// This is used by DT_FOREACH_PROP_ELEM below to build the adc_channels array
#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

// Array of ADC channel specifications from devicetree
// This is populated automatically from the overlay file which defines 4 channels (A0-A3)
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

/* 
 * XIAO nRF54L15 has 4 accessible analog input pins:
 * 
 * Array Index | Header Pin | GPIO Pin | ADC Channel | Usage in this sample
 * ------------|------------|----------|-------------|---------------------
 *      0      |     A0     |  P1.04   |    AIN0     | Potentiometer input
 *      1      |     A1     |  P1.05   |    AIN1     | (unused)
 *      2      |     A2     |  P1.06   |    AIN2     | (unused)
 *      3      |     A3     |  P1.07   |    AIN3     | (unused)
 * 
 * To use a different pin, change this index (0-3)
 */
#define POTENTIOMETER_ADC_CHANNEL_IDX 0

// ============================================================================
// LED CONFIGURATION
// ============================================================================

// Get the devicetree node for the built-in LED
// "led0" is an alias defined in the board's devicetree files
#define LED0_NODE DT_ALIAS(led0)

// Get the GPIO specification (port, pin, flags) for the LED
// This LED is on P2.0 (GPIO port 2, pin 0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// ============================================================================
// TIMING CONFIGURATION
// ============================================================================

// Minimum blink delay (when voltage is at maximum)
#define MIN_BLINK_MS 1

// Maximum blink delay (when voltage is at minimum)
// Set to 3330 to match the typical max voltage (3.3V = 3300mV)
#define MAX_BLINK_MS 3330

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(void)
{
    // Return value for error checking (negative = error, 0 or positive = success)
    int ret;
    
    // Variable to store the raw ADC reading (0-4095 for 12-bit resolution)
    uint16_t adc_raw_value;
    
    // Variable to store the voltage in millivolts (mV)
    // Will be automatically calculated from raw value
    int32_t adc_millivolts;

    // Print startup message to console
    LOG_INF("Starting ADC Blink Example...");
    LOG_INF("Connect potentiometer to A0 (P1.04)");
    LOG_INF("Turn LEFT for slow blink, RIGHT for fast blink");

    // ========================================================================
    // STEP 1: Initialize the ADC
    // ========================================================================
    
    // Check if the ADC device is ready to use
    if (!adc_is_ready_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX])) {
        LOG_ERR("ADC controller device %s not ready", 
                adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].dev->name);
        return 0;  // Exit if ADC is not available
    }

    // Configure the ADC channel with settings from devicetree
    // (resolution, gain, reference voltage, etc.)
    ret = adc_channel_setup_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX]);
    if (ret < 0) {
        LOG_ERR("Could not setup ADC channel for potentiometer (%d)", ret);
        return 0;  // Exit if setup failed
    }
    
    LOG_INF("✓ ADC device %s, channel %d ready",
            adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].dev->name,
            adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].channel_id);

    // ========================================================================
    // STEP 2: Initialize the LED
    // ========================================================================
    
    // Check if the GPIO port for the LED is ready
    if (!gpio_is_ready_dt(&led)) {
        LOG_ERR("LED GPIO device is not ready");
        return 0;  // Exit if GPIO is not available
    }
    
    // Configure the LED pin as an output, starting in the OFF state
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure LED GPIO (%d)", ret);
        return 0;  // Exit if configuration failed
    }
    
    LOG_INF("✓ LED ready on P2.0");

    // ========================================================================
    // STEP 3: Configure ADC Reading Sequence
    // ========================================================================
    
    /*
     * An ADC sequence tells the ADC driver:
     * - Where to store the result (buffer)
     * - How big the buffer is (buffer_size)
     * - What resolution to use (resolution, e.g., 12-bit = 0-4095)
     * 
     * This structure is initialized once and reused for each reading
     */
    struct adc_sequence sequence = {
        .buffer = &adc_raw_value,           // Store result in this variable
        .buffer_size = sizeof(adc_raw_value), // Size = 2 bytes (uint16_t)
        .resolution = adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].resolution, // 12 bits
    };

    // ========================================================================
    // STEP 4: Main Loop - Read ADC and Control LED
    // ========================================================================
    
    LOG_INF("Starting main loop...\n");
    
    while (1) {  // Infinite loop - runs forever
        
        // ====================================================================
        // Read the ADC value
        // ====================================================================
        
        // Initialize the sequence with channel-specific settings
        // This sets up things like which ADC input to read
        (void)adc_sequence_init_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX], &sequence);

        // Perform the actual ADC reading
        // The result will be stored in adc_raw_value (0-4095 for 12-bit)
        ret = adc_read(adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].dev, &sequence);
        if (ret < 0) {
            // If reading failed, log error and try again after 100ms
            LOG_ERR("ADC read failed: %d", ret);
            k_msleep(100);
            continue;  // Skip to next loop iteration
        }

        // Copy the raw value for display purposes
        int sensor_value = adc_raw_value;

        // ====================================================================
        // Convert raw ADC value to millivolts
        // ====================================================================
        
        /*
         * The ADC gives us a raw number (e.g., 2048 out of 4095)
         * We need to convert this to actual voltage in millivolts
         * 
         * The conversion accounts for:
         * - Reference voltage (internal or external)
         * - ADC gain setting (typically 1/4 for 0-3.3V range)
         * - Resolution (12-bit = 4096 possible values)
         * 
         * Example: If raw = 4095 (max), result might be 3300mV (3.3V)
         */
        adc_millivolts = sensor_value;
        ret = adc_raw_to_millivolts_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX], 
                                       &adc_millivolts);
        if (ret < 0) {
            // Conversion not supported on this platform - shouldn't happen
            LOG_WRN("Voltage conversion failed: %d", ret);
            k_msleep(100);
            continue;
        }

        // ====================================================================
        // Calculate blink delay from voltage
        // ====================================================================
        
        /*
         * INVERTED MAPPING:
         * - Low voltage (0mV)    -> Long delay (3330ms)  -> SLOW blink
         * - High voltage (3.3V)  -> Short delay (1ms)    -> FAST blink
         * 
         * Formula: delay = MAX - voltage + 1
         * 
         * Examples:
         *   0mV    -> 3330 - 0 + 1    = 3331ms (very slow)
         *   1650mV -> 3330 - 1650 + 1 = 1681ms (medium)
         *   3330mV -> 3330 - 3330 + 1 = 1ms    (very fast)
         */
        
        // Safety check: Clamp voltage to valid range to prevent integer underflow
        // (Sometimes ADC can read slightly above 3.3V due to noise)
        if (adc_millivolts > MAX_BLINK_MS) {
            adc_millivolts = MAX_BLINK_MS;  // Cap at 3330mV
        }
        
        // Calculate the blink delay using inverted formula
        uint32_t blink_delay_ms = MAX_BLINK_MS - adc_millivolts + 1;

        // ====================================================================
        // Toggle the LED
        // ====================================================================
        
        /*
         * gpio_pin_toggle_dt() changes the LED state:
         * - If LED is OFF, turn it ON
         * - If LED is ON, turn it OFF
         * 
         * By calling this in a loop with a delay, we create a blinking effect
         */
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            LOG_ERR("Failed to toggle LED: %d", ret);
        }

        // ====================================================================
        // Print debug information
        // ====================================================================
        
        // Show raw ADC value, converted voltage, and calculated delay
        // This appears in the serial console/terminal
        LOG_INF("ADC Raw = %4d  |  Voltage = %4d mV  |  Blink Delay = %4u ms",
                sensor_value, adc_millivolts, blink_delay_ms);

        // ====================================================================
        // Wait before next reading
        // ====================================================================
        
        /*
         * Sleep for the calculated delay
         * This serves two purposes:
         * 1. Creates the visible blink effect
         * 2. Controls how fast the LED toggles based on pot position
         * 
         * k_msleep() puts the CPU to sleep to save power
         */
        k_msleep(blink_delay_ms);
        
    }  // End of infinite loop
    
    // This line is never reached (infinite loop above)
    return 0;
}