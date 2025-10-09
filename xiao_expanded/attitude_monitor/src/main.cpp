#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <nrfx_power.h>

#include "../../../lib/MadgwickAHRS/src/MadgwickAHRS.h"   // Centralized Arduino Madgwick AHRS library
#include "../../../lib/u8g2/csrc/u8g2.h"                    // Centralized U8g2 library
#include "../../../lib/u8g2/csrc/u8x8.h"                    // Centralized U8x8 library

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
// IMU LSM6DSO definitions and functions
// ****************************************************************************************************
const struct device *i2c30_dev = DEVICE_DT_GET(DT_NODELABEL(i2c30));

#define LSM6DSO_I2C_ADDR    0x6A    // LSM6DSO I2C device address
#define LSM6DSO_REG_WHO_AM_I 0x0F   // Identification register
#define LSM6DSO_WHO_AM_I_VAL 0x6A   // Expected WHO_AM_I value
#define LSM6DSO_REG_CTRL1_XL 0x10   // Accelerometer control register
#define LSM6DSO_REG_CTRL2_G  0x11   // Gyroscope control register
#define LSM6DSO_REG_CTRL8_XL 0x17   // Accelerometer control register
#define LSM6DSO_REG_CTRL7_G  0x16   // Gyroscope control register
#define LSM6DSO_REG_OUTX_L_XL 0x28  // Accelerometer X axis low byte(low byte first)
#define LSM6DSO_REG_OUTX_L_G  0x22  // Gyroscope X axis low byte(low byte first)
#define LSM6DSO_REG_STATUS    0x1E  // Status data register for user interface

// Write a single byte to an LSM6DSO register via I2C.
static int lsm6dso_i2c_reg_write_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t value) {
    uint8_t tx_buf[2] = {reg_addr, value};
    return i2c_write(i2c_dev, tx_buf, sizeof(tx_buf), LSM6DSO_I2C_ADDR);
}
// Read a single byte from an LSM6DSO register via I2C.
static int lsm6dso_i2c_reg_read_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *value) {
    return i2c_reg_read_byte(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, value);
}
// Read multiple consecutive bytes from LSM6DSO register via I2C.
static int lsm6dso_i2c_reg_read_bytes(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *data, uint8_t len) {
    return i2c_burst_read(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, data, len);
}

// *************************************************************************************************
// U8g2 definition and functions
// *************************************************************************************************
const struct device *i2c22_dev = DEVICE_DT_GET(DT_NODELABEL(i2c22));

// call back function for u8x8
#define I2C_BUFFER_SIZE 64
static uint8_t i2c_buf[I2C_BUFFER_SIZE];
static uint8_t i2c_buf_len = 0;

uint8_t u8x8_byte_zephyr_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {    
    switch (msg) {
    case U8X8_MSG_BYTE_INIT:
        return 1;
    case U8X8_MSG_BYTE_START_TRANSFER:
        i2c_buf_len = 0;
        return 1;
    case U8X8_MSG_BYTE_SEND:
        if (i2c_buf_len + arg_int > I2C_BUFFER_SIZE)
            return 0;
        memcpy(&i2c_buf[i2c_buf_len], arg_ptr, arg_int);
        i2c_buf_len += arg_int;
        return 1;
    case U8X8_MSG_BYTE_END_TRANSFER:
        i2c_write(i2c22_dev, i2c_buf, i2c_buf_len,
                  u8x8_GetI2CAddress(u8x8) >> 1);
        return 1;
    }
    return 0; 
}

uint8_t u8x8_gpio_and_delay_zephyr(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    k_sleep(K_MSEC(arg_int));
    return 1;
}


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
    Madgwick filter;
    filter.begin(12.5);   
    LOG_INF("Madgwick filter sampling rate : 12.5\n");

    // -----------------------------------------------------------------------------
    //                           IMU LSM6DSO setup
    // -------------------------------------------------------------------------------
    if (!device_is_ready(i2c30_dev)) {
        LOG_ERR("I2C30 device %s is not ready!\n", i2c30_dev->name);
        return 0;
    }
    LOG_INF("I2C30 device %s is ready\n", i2c30_dev->name);

    // Verify device ID
    uint8_t who_am_i = 0;
    ret = lsm6dso_i2c_reg_read_byte(i2c30_dev, LSM6DSO_REG_WHO_AM_I, &who_am_i);
    if (ret != 0) {
        LOG_ERR("Failed to read WHO_AM_I register (err: %d)\n", ret);
        return ret;
    }
    if (who_am_i != LSM6DSO_WHO_AM_I_VAL) {
        LOG_ERR("Invalid WHO_AM_I: 0x%02x, expected 0x%02x\n", who_am_i, LSM6DSO_WHO_AM_I_VAL);
        return -ENODEV;
    }
    LOG_INF("LSM6DSO WHO_AM_I check passed. ID: 0x%02x\n", who_am_i);

    // Set accelerometer ODR (12.5 Hz) and ±4g range (0x1A)
    ret = lsm6dso_i2c_reg_write_byte(i2c30_dev, LSM6DSO_REG_CTRL1_XL, 0x1A);
    if (ret != 0) {
        LOG_ERR("Failed to set CTRL1_XL register (err: %d)\n", ret);
        return ret;
    }
    // Set gyroscope ODR (12.5 Hz) and ±2000dps range (0x1C)
    ret = lsm6dso_i2c_reg_write_byte(i2c30_dev, LSM6DSO_REG_CTRL2_G, 0x1C); 
    if (ret != 0) {
        LOG_ERR("Failed to set CTRL2_G register (err: %d)\n", ret);
        return ret;
    }
    // Set the ODR config register to ODR/4
    ret = lsm6dso_i2c_reg_write_byte(i2c30_dev, LSM6DSO_REG_CTRL8_XL, 0x09);
    if (ret != 0) {
        LOG_ERR("Failed to set CTRL8_XL register (err: %d)\n", ret);
        return ret;
    }
    // set gyroscope power mode to high performance and bandwidth to 16 mHz
    ret = lsm6dso_i2c_reg_write_byte(i2c30_dev, LSM6DSO_REG_CTRL7_G, 0x00); 
    if (ret != 0) {
        LOG_ERR("Failed to set CTRL7_G register (err: %d)\n", ret);
        return ret;
    }
    // registers value check
    uint8_t value;
    lsm6dso_i2c_reg_read_byte(i2c30_dev, LSM6DSO_REG_CTRL1_XL, &value);
    LOG_INF("CTRL1_XL = 0x%02X\n", value);
    lsm6dso_i2c_reg_read_byte(i2c30_dev, LSM6DSO_REG_CTRL2_G, &value);
    LOG_INF("CTRL2_G = 0x%02X\n", value);
    lsm6dso_i2c_reg_read_byte(i2c30_dev, LSM6DSO_REG_CTRL8_XL, &value);
    LOG_INF("CTRL8_XL = 0x%02X\n", value);
    lsm6dso_i2c_reg_read_byte(i2c30_dev, LSM6DSO_REG_CTRL7_G, &value);
    LOG_INF("CTRL7_G = 0x%02X\n", value);

    LOG_INF("LSM6DSO initialized successfully\n");

    // --------------------------------------------------------------------------------------
    //                           Oled Display SSD1306 setup
    // --------------------------------------------------------------------------------------
//	i2c22_dev = DEVICE_DT_GET(DT_NODELABEL(i2c22));
    if (!device_is_ready(i2c22_dev)) {
        LOG_ERR("I2C22 device %s is not ready!\n", i2c22_dev->name);
        return -1;
    }

    u8g2_t u8g2;
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &u8g2, U8G2_R0, u8x8_byte_zephyr_hw_i2c, u8x8_gpio_and_delay_zephyr);

    u8g2_SetI2CAddress(&u8g2, 0x3c << 1);
    u8g2_InitInterface(&u8g2);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);    // ON
	u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);    // black screen with white text

//    u8g2_SetFont(&u8g2, u8g2_font_timB14_tr);
//    u8g2_SetFont(&u8g2, u8g2_font_chargen_92_tr);
    u8g2_SetFont(&u8g2, u8g2_font_ncenR14_tr);
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawStr(&u8g2, 0, 13, "  Attitude");
    u8g2_DrawStr(&u8g2, 0, 29, "        Monitor");
//    u8g2_SetFont(&u8g2, u8g2_font_timR14_tr);    
//    u8g2_DrawStr(&u8g2, 0, 45, "   nRF54L15");
    u8g2_SetFont(&u8g2, u8g2_font_courR10_tr);
    u8g2_DrawStr(&u8g2, 0, 60, " XIAO_nRF54L15");        
    u8g2_SendBuffer(&u8g2);
    k_msleep(5000);

	u8g2_SetFont(&u8g2, u8g2_font_9x15_tr);     // 14 characters / LINE


// --------------------------------------------------------------------------------
// loop
// --------------------------------------------------------------------------------------
    while (1) {
ret = gpio_pin_set_dt(&test0, true);                    // test-pin HIGH        
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

        // acc and gyro read
        uint8_t accel_data[6];
        uint8_t gyro_data[6];
        uint8_t status = 0;
        int16_t iax, iay, iaz, igx, igy, igz;
        float   fax, fay, faz, fgx, fgy, fgz;
ret = gpio_pin_set_dt(&test0, false);
        // wait for ready to read
        do {  
            lsm6dso_i2c_reg_read_byte(i2c30_dev, LSM6DSO_REG_STATUS, &status);   //0,0,0,0,0,TDA,GDA,XLDA; 
        } while ((status & 0x07) != 0x07);
ret = gpio_pin_set_dt(&test0, true);
        // Read accelerometer data (6 bytes)
        err = lsm6dso_i2c_reg_read_bytes(i2c30_dev, LSM6DSO_REG_OUTX_L_XL, accel_data, 6);
        if (err != 0) {
            LOG_ERR("Failed to read accelerometer data (err: %d)\n", err);
            return err;
        }
        // Raw data is 16-bit signed integer, low byte first
        iax = (int16_t)(accel_data[0] | (accel_data[1] << 8));
        iay = (int16_t)(accel_data[2] | (accel_data[3] << 8));
        iaz = (int16_t)(accel_data[4] | (accel_data[5] << 8));   

        // Read gyroscope data (6 bytes)
        err = lsm6dso_i2c_reg_read_bytes(i2c30_dev, LSM6DSO_REG_OUTX_L_G, gyro_data, 6);
        if (err != 0) {
            LOG_ERR("Failed to read gyroscope data (err: %d)\n", err);
            return err;
        }
        // Raw data is 16-bit signed integer, low byte first
        igx = (int16_t)(gyro_data[0] | (gyro_data[1] << 8));
        igy = (int16_t)(gyro_data[2] | (gyro_data[3] << 8));
        igz = (int16_t)(gyro_data[4] | (gyro_data[5] << 8));

        float accel_scale = 4.0f / 32768.0f;
        fax = iax * accel_scale;
        fay = iay * accel_scale;
        faz = iaz * accel_scale;
        float gyro_scale = 2000.0f / 32768.0f;
        fgx = igx * gyro_scale;
        fgy = igy * gyro_scale;
        fgz = igz * gyro_scale;
        LOG_INF("accel float: X:%2.2f Y:%2.2f Z:%2.2f (LSB)\n", fax, fay, faz);
        LOG_INF("gyro float: X:%2.2f Y:%2.2f Z:%2.2f (LSB)\n", fgx, fgy, fgz);

        filter.updateIMU(fgx, fgy, fgz, fax, fay, faz);
        float roll = filter.getRoll();
        float pitch = filter.getPitch();
        float yaw = filter.getYaw();
        LOG_INF("%3.2f, %3.2f, %3.2f, %3.2f\n", roll, pitch, yaw, val_mv*2/1000.0f);

        // Read usrbtn or selsw state
        int usrbtn_state = gpio_pin_get_dt(&usrbtn);
        int selsw_state = gpio_pin_get_dt(&selsw);

        // Display data
        char buf[16];
        if (selsw_state == 1 || usrbtn_state == 1) { // Button pressed (ACTIVE_LOW)
		    u8g2_ClearBuffer(&u8g2);
		    snprintf(buf, sizeof(buf), "%7.2f%7.2f", fax, fgx);
		    u8g2_DrawStr(&u8g2, 0, 13, buf);
		    snprintf(buf, sizeof(buf), "%7.2f%7.2f", fay, fgy);
		    u8g2_DrawStr(&u8g2, 0, 29, buf);
            snprintf(buf, sizeof(buf), "%7.2f%7.2f", faz, fgz);
            u8g2_DrawStr(&u8g2, 0, 45, buf);
            snprintf(buf, sizeof(buf), "Bat %4.2f %5d", val_mv/500.0f, timestamp/1000);
            u8g2_DrawStr(&u8g2, 0, 61, buf);
            u8g2_SendBuffer(&u8g2);
        } else {
		    u8g2_ClearBuffer(&u8g2);
		    snprintf(buf, sizeof(buf), "Roll   %7.2f", roll);
		    u8g2_DrawStr(&u8g2, 0, 13, buf);
		    snprintf(buf, sizeof(buf), "Pitch  %7.2f", pitch);
		    u8g2_DrawStr(&u8g2, 0, 29, buf);
            snprintf(buf, sizeof(buf), "Yaw    %7.2f", yaw);
            u8g2_DrawStr(&u8g2, 0, 45, buf);
            snprintf(buf, sizeof(buf), "Bat %4.2f %5d", val_mv/500.0f, timestamp/1000);
            u8g2_DrawStr(&u8g2, 0, 61, buf);
            u8g2_SendBuffer(&u8g2);
        }
        ret = gpio_pin_set_dt(&led_builtin, true);  // LED OFF
ret = gpio_pin_set_dt(&test0, false);               // test pin LOW, loop 39 mS (display 36 mS)
        while((uint32_t)k_uptime_get() - timestamp < 80);
    }
    return 0;
}