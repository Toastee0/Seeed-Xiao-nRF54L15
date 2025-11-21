#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/devicetree.h>
#include <nrfx_power.h>

#include <lvgl.h>

#include "madgwick_cmsis.h"   // CMSIS-DSP based Madgwick filter

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main_app, CONFIG_LOG_DEFAULT_LEVEL);

// *****************************************************************************************************
// Ports structures
// *****************************************************************************************************
static const struct gpio_dt_spec led_builtin = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec usrbtn = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec test0 = GPIO_DT_SPEC_GET(DT_ALIAS(p104d0), gpios);
static const struct gpio_dt_spec selsw = GPIO_DT_SPEC_GET(DT_ALIAS(p105d1), gpios);

// ****************************************************************************************************
// ADC definition and structures
// ****************************************************************************************************
#define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

// Data of ADC io-channels specified in devicetree.
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)};

static const struct device *const vbat_reg = DEVICE_DT_GET(DT_NODELABEL(vbat_pwr));

// ***************************************************************************************************
// IMU LSM6DSO sensor device
// ****************************************************************************************************
const struct device *const imu_dev = DEVICE_DT_GET(DT_ALIAS(accel0));

// *************************************************************************************************
// LVGL Display Objects
// *************************************************************************************************
static lv_obj_t *label_line1;
static lv_obj_t *label_line2;
static lv_obj_t *label_line3;
static lv_obj_t *label_line4;


// ***************************************************************************************
// ************************************** Main function **********************************
// ***************************************************************************************
int main(void)
{
    int ret;
	int err;

    // ------------------------------------------------------------------------------
    //                                      GPIO setup
    // ------------------------------------------------------------------------------
    if(!gpio_is_ready_dt(&led_builtin) || !gpio_is_ready_dt(&usrbtn)
                                       || !gpio_is_ready_dt(&test0)
                                       || !gpio_is_ready_dt(&selsw)) {
        LOG_ERR("Some Ports are not ready!\n");
        return 0;
    }

    err = gpio_pin_configure_dt(&led_builtin, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
        LOG_ERR("Could not setup led_builtin (%d)\n", err);
		return err;
	}
    err = gpio_pin_configure_dt(&usrbtn, GPIO_INPUT);
    if (err < 0) {
        LOG_ERR("Could not setup usrbtn (%d)\n", err);
		return err;
    }
    err = gpio_pin_configure_dt(&test0, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
        LOG_ERR("Could not setup test0 (%d)\n", err);
		return err;
	}
    err = gpio_pin_configure_dt(&selsw, GPIO_INPUT);
    if (err < 0) {
        LOG_ERR("Could not setup selsw (%d)\n", err);
		return err;
    }
    
    LOG_INF("Ports initialized successfully\n");

    // -------------------------------------------------------------------------------
    //                                   Vbat read adc setup
    // ------------------------------------------------------------------------------
	uint16_t buf;
	int32_t val_mv;
	struct adc_sequence sequence = {
		.buffer = &buf,
		.buffer_size = sizeof(buf),
	};

//	printk("vbat enable\n");
	regulator_enable(vbat_reg);	// P1.15/VBAT_EN = HIGH
	k_msleep(100);
    LOG_INF("Vbat Read enable\n");

	// Configure channels individually prior to sampling.
	if (!adc_is_ready_dt(&adc_channels[7]))
	{
		LOG_ERR("ADC controller device %s not ready\n", adc_channels[7].dev->name);
		return 0;
	}

	err = adc_channel_setup_dt(&adc_channels[7]);
	if (err < 0)
	{
		LOG_ERR("Could not setup channel #7 (%d)\n", err);
		return 0;
	}

	(void)adc_sequence_init_dt(&adc_channels[7], &sequence);

    // ------------------------------------------------------------------------------
    //                        set Madgwick filter sampling rate
    // -------------------------------------------------------------------------------
    MadgwickFilter filter;
    filter.begin(12.5);
    LOG_INF("Madgwick filter sampling rate : 12.5Hz\n");

    // -----------------------------------------------------------------------------
    //                           IMU LSM6DSO setup
    // -------------------------------------------------------------------------------
    if (!device_is_ready(imu_dev)) {
        LOG_ERR("IMU device not ready\n");
        return 0;
    }
    LOG_INF("IMU device ready\n");

    // Set accelerometer range to ±4g (to match original config)
    struct sensor_value accel_range;
    sensor_g_to_ms2(4, &accel_range);  // 4g range

    err = sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ,
                SENSOR_ATTR_FULL_SCALE, &accel_range);
    if (err < 0 && err != -ENOTSUP && err != -ENOSYS) {
        LOG_ERR("Cannot set accelerometer range: %d\n", err);
        return 0;
    }

    // Set gyroscope range to ±2000dps (to match original config)
    struct sensor_value gyro_range;
    sensor_degrees_to_rad(2000, &gyro_range);  // 2000 dps range

    err = sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ,
                SENSOR_ATTR_FULL_SCALE, &gyro_range);
    if (err < 0 && err != -ENOTSUP && err != -ENOSYS) {
        LOG_ERR("Cannot set gyroscope range: %d\n", err);
        return 0;
    }

    // Set accelerometer sampling frequency to 12.5 Hz
    struct sensor_value odr_attr;
    odr_attr.val1 = 12;
    odr_attr.val2 = 500000;  // 12.5 Hz

    err = sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ,
                SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
    if (err < 0 && err != -ENOTSUP && err != -ENOSYS) {
        LOG_ERR("Cannot set sampling frequency for accelerometer: %d\n", err);
        return 0;
    }

    err = sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ,
                SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
    if (err < 0 && err != -ENOTSUP && err != -ENOSYS) {
        LOG_ERR("Cannot set sampling frequency for gyro: %d\n", err);
        return 0;
    }

    LOG_INF("LSM6DSL initialized: \u00b14g, \u00b12000dps, 12.5 Hz\n");

    // --------------------------------------------------------------------------------------
    //                           LVGL Display SSD1306 setup
    // --------------------------------------------------------------------------------------
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Display device not ready\n");
        return -1;
    }
    LOG_INF("Display device ready\n");

    // Get the active screen (don't create a new one)
    lv_obj_t *screen = lv_screen_active();

    // Create labels for 4 lines of text (matching the original layout)
    label_line1 = lv_label_create(screen);
    lv_obj_align(label_line1, LV_ALIGN_TOP_LEFT, 0, 0);

    label_line2 = lv_label_create(screen);
    lv_obj_align(label_line2, LV_ALIGN_TOP_LEFT, 0, 16);

    label_line3 = lv_label_create(screen);
    lv_obj_align(label_line3, LV_ALIGN_TOP_LEFT, 0, 32);

    label_line4 = lv_label_create(screen);
    lv_obj_align(label_line4, LV_ALIGN_TOP_LEFT, 0, 48);

    // Display title screen
    lv_label_set_text(label_line1, "  Attitude");
    lv_label_set_text(label_line2, "    Monitor");
    lv_label_set_text(label_line3, "");
    lv_label_set_text(label_line4, "XIAO nRF54L15");

    lv_task_handler();
    display_blanking_off(display_dev);

    LOG_INF("Display initialized\n");
    k_msleep(3000);


// --------------------------------------------------------------------------------
// loop
// --------------------------------------------------------------------------------------
    uint32_t loop_count = 0;
    while (1) {
        ret = gpio_pin_set_dt(&test0, true);             // test-pin HIGH
        ret = gpio_pin_set_dt(&led_builtin, false);     // LED ON
        uint32_t timestamp = (uint32_t)k_uptime_get();

        // Vbat read
		err = adc_read_dt(&adc_channels[7], &sequence);
		if (err < 0) {
			LOG_ERR("Could not read (%d)\n", err);
			return 0;
		}

		if (adc_channels[7].channel_cfg.differential) {
			val_mv = (int32_t)((int16_t)buf);
		} else {
			val_mv = (int32_t)buf;
		}

		// conversion to mV may not be supported, skip if not
		err = adc_raw_to_millivolts_dt(&adc_channels[7], &val_mv);
		if (err < 0) {
			LOG_ERR(" value in mV not available\n");
		} else {
			LOG_INF("bat vol = %" PRId32 " mV\n", val_mv * 2);
		}

        // acc and gyro read using sensor API
        struct sensor_value accel[3], gyro[3];
        float fax, fay, faz, fgx, fgy, fgz;

        ret = gpio_pin_set_dt(&test0, false);

        // Fetch new sensor data
        ret = sensor_sample_fetch(imu_dev);
        if (ret < 0) {
            LOG_ERR("sensor_sample_fetch() failed: %d\n", ret);
            return ret;
        }

        ret = gpio_pin_set_dt(&test0, true);

        // Get accelerometer data
        ret = sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
        if (ret < 0) {
            LOG_ERR("Failed to get accel: %d\n", ret);
            return ret;
        }

        // Get gyroscope data
        ret = sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_XYZ, gyro);
        if (ret < 0) {
            LOG_ERR("Failed to get gyro: %d\n", ret);
            return ret;
        }

        // Convert sensor_value to float
        fax = sensor_value_to_float(&accel[0]);
        fay = sensor_value_to_float(&accel[1]);
        faz = sensor_value_to_float(&accel[2]);
        fgx = sensor_value_to_float(&gyro[0]);
        fgy = sensor_value_to_float(&gyro[1]);
        fgz = sensor_value_to_float(&gyro[2]);

        // Convert accelerometer from m/s² to g-units (1g = 9.80665 m/s²)
        fax /= 9.80665f;
        fay /= 9.80665f;
        faz /= 9.80665f;

        // Convert gyro from rad/s to deg/s for Madgwick filter
        // (Madgwick expects deg/s, sensor API returns rad/s)
        fgx *= 57.2958f;  // 180/π
        fgy *= 57.2958f;
        fgz *= 57.2958f;

        LOG_INF("accel float: X:%2.2f Y:%2.2f Z:%2.2f (g)\n", fax, fay, faz);
        LOG_INF("gyro float: X:%2.2f Y:%2.2f Z:%2.2f (deg/s)\n", fgx, fgy, fgz);

        filter.updateIMU(fgx, fgy, fgz, fax, fay, faz);
        float roll = filter.getRoll();
        float pitch = filter.getPitch();
        float yaw = filter.getYaw();
        LOG_INF("%3.2f, %3.2f, %3.2f, %3.2f\n", roll, pitch, yaw, val_mv*2/1000.0f);

        // Read usrbtn or selsw state
        int usrbtn_state = gpio_pin_get_dt(&usrbtn);
        int selsw_state = gpio_pin_get_dt(&selsw);

        // Display data
        char buf1[32], buf2[32], buf3[32], buf4[32];
        if (selsw_state == 1 || usrbtn_state == 1) { // Button pressed - show raw data
            snprintf(buf1, sizeof(buf1), "%7.2f%7.2f", fax, fgx);
            snprintf(buf2, sizeof(buf2), "%7.2f%7.2f", fay, fgy);
            snprintf(buf3, sizeof(buf3), "%7.2f%7.2f", faz, fgz);
            snprintf(buf4, sizeof(buf4), "Bat %4.2f %5d", val_mv/500.0f, timestamp/1000);

            lv_label_set_text(label_line1, buf1);
            lv_label_set_text(label_line2, buf2);
            lv_label_set_text(label_line3, buf3);
            lv_label_set_text(label_line4, buf4);
        } else { // Show attitude data
            snprintf(buf1, sizeof(buf1), "Roll   %7.2f", roll);
            snprintf(buf2, sizeof(buf2), "Pitch  %7.2f", pitch);
            snprintf(buf3, sizeof(buf3), "Yaw    %7.2f", yaw);
            snprintf(buf4, sizeof(buf4), "Bat %4.2f %5d", val_mv/500.0f, timestamp/1000);

            lv_label_set_text(label_line1, buf1);
            lv_label_set_text(label_line2, buf2);
            lv_label_set_text(label_line3, buf3);
            lv_label_set_text(label_line4, buf4);
        }

        ret = gpio_pin_set_dt(&led_builtin, true);  // LED OFF
        ret = gpio_pin_set_dt(&test0, false);        // test pin LOW

        // Update LVGL display
        lv_task_handler();

        // Log every 10th iteration to avoid flooding
        loop_count++;
        if (loop_count % 10 == 0) {
            LOG_INF("Loop %d: R:%.1f P:%.1f Y:%.1f Bat:%.2fV",
                    loop_count, roll, pitch, yaw, val_mv*2/1000.0f);
        }

        // Sleep for remaining time to maintain ~80ms loop time
        uint32_t elapsed = (uint32_t)k_uptime_get() - timestamp;
        if (elapsed < 80) {
            k_msleep(80 - elapsed);
        }
    }
    return 0;
}