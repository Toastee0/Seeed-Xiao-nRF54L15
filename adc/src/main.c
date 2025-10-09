#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

// Register log module
LOG_MODULE_REGISTER(pot_pwm_example, CONFIG_LOG_DEFAULT_LEVEL);

// --- ADC Configuration ---
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified for ADC channels"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

// Define the index of the potentiometer ADC channel in the adc_channels array
#define POTENTIOMETER_ADC_CHANNEL_IDX 1

// --- PWM Configuration ---
// Get PWM LED device
static const struct pwm_dt_spec led = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led));

// Define PWM period as 1 millisecond (1,000,000 nanoseconds)
// This corresponds to a 1 kHz PWM frequency, suitable for LED brightness adjustment without visible flicker
#define PWM_PERIOD_NS 1000000UL

int main(void)
{
    int ret;
    uint16_t adc_raw_value;
    int32_t adc_millivolts;

    LOG_INF("Starting Zephyr Potentiometer to PWM example...");

    // --- ADC initialization and setup ---
    if (!adc_is_ready_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX])) {
        LOG_ERR("ADC controller device %s not ready", adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].dev->name);
        return 0;
    }

    ret = adc_channel_setup_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX]);
    if (ret < 0) {
        LOG_ERR("Could not setup ADC channel for potentiometer (%d)", ret);
        return 0;
    }
    LOG_INF("ADC device %s, channel %d ready for potentiometer.",
            adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].dev->name,
            adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].channel_id);

    // --- PWM initialization and setup ---
    if (!device_is_ready(led.dev)) {
        LOG_ERR("Error: PWM device %s is not ready", led.dev->name);
        return 0;
    }
    LOG_INF("PWM Period for LED set to %lu ns (%.1f Hz)",
            PWM_PERIOD_NS, (double)NSEC_PER_SEC / PWM_PERIOD_NS); // Use PWM_PERIOD_NS instead of led.period


    // ADC sequence configuration
    struct adc_sequence sequence = {
        .buffer = &adc_raw_value,
        .buffer_size = sizeof(adc_raw_value),
        .resolution = adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].resolution,
    };

    // --- Main loop ---
    while (1) {
        (void)adc_sequence_init_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX], &sequence);

        ret = adc_read(adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].dev, &sequence);
        if (ret < 0) {
            LOG_ERR("Error %d: ADC read failed for channel %d",
                    ret, adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].channel_id);
            k_msleep(100);
            continue;
        }

        int sensor_value = adc_raw_value;

        uint32_t max_adc_raw = (1U << adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX].resolution) - 1;

        // --- Map ADC raw value to PWM duty cycle ---
        uint32_t output_duty_ns = (PWM_PERIOD_NS * sensor_value) / max_adc_raw;

        // Set PWM duty cycle
        ret = pwm_set_dt(&led, PWM_PERIOD_NS, output_duty_ns);
        if (ret < 0) {
            LOG_ERR("Error %d: failed to set PWM duty cycle.", ret);
        }

        // --- Print information ---
        adc_millivolts = sensor_value;
        ret = adc_raw_to_millivolts_dt(&adc_channels[POTENTIOMETER_ADC_CHANNEL_IDX], &adc_millivolts);
        if (ret < 0) {
            LOG_WRN("ADC to mV conversion not supported/failed: %d", ret);
            LOG_INF("Sensor Raw Value = %d\tOutput Duty (ns) = %u", sensor_value, output_duty_ns);
        } else {
            LOG_INF("Sensor Raw Value = %d (%d mV)\tOutput Duty (ns) = %u",
                    sensor_value, adc_millivolts, output_duty_ns);
        }

        k_msleep(100);
    }
    return 0;
}