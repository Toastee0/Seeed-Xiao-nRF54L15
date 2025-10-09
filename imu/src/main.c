#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(lsm6dso_i2c_example, LOG_LEVEL_INF);

// --- LSM6DSO I2C address and register definitions ---
#define LSM6DSO_I2C_ADDR    0x6A // LSM6DSO I2C device address

#define LSM6DSO_REG_WHO_AM_I 0x0F // Identification register
#define LSM6DSO_WHO_AM_I_VAL 0x6A // Expected WHO_AM_I value

#define LSM6DSO_REG_CTRL1_XL 0x10 // Accelerometer control register
#define LSM6DSO_REG_CTRL2_G  0x11 // Gyroscope control register
// Accelerometer/gyroscope data output registers (low byte first)
#define LSM6DSO_REG_OUTX_L_XL 0x28 // Accelerometer X axis low byte
#define LSM6DSO_REG_OUTX_L_G  0x22 // Gyroscope X axis low byte

// --- Data structure definitions ---
// Structure for storing raw sensor data
struct lsm6dso_raw_data {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

// --- Helper functions ---

/**
 * @brief Write a single byte to an LSM6DSO register via I2C.
 */
static int lsm6dso_i2c_reg_write_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t value)
{
    uint8_t tx_buf[2] = {reg_addr, value};
    return i2c_write(i2c_dev, tx_buf, sizeof(tx_buf), LSM6DSO_I2C_ADDR);
}

/**
 * @brief Read a single byte from an LSM6DSO register via I2C.
 */
static int lsm6dso_i2c_reg_read_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *value)
{
    return i2c_reg_read_byte(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, value);
}

/**
 * @brief Read multiple consecutive bytes from LSM6DSO register via I2C.
 */
static int lsm6dso_i2c_reg_read_bytes(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    return i2c_burst_read(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, data, len);
}

// --- LSM6DSO driver core functionality ---

/**
 * @brief Initialize the LSM6DSO sensor.
 * Check WHO_AM_I and set ODR for accelerometer and gyroscope.
 */
static int lsm6dso_init(const struct device *i2c_dev)
{
    uint8_t who_am_i = 0;
    int ret;

    // Verify device ID
    ret = lsm6dso_i2c_reg_read_byte(i2c_dev, LSM6DSO_REG_WHO_AM_I, &who_am_i);
    if (ret != 0) {
        LOG_ERR("Failed to read WHO_AM_I register (err: %d)", ret);
        return ret;
    }
    if (who_am_i != LSM6DSO_WHO_AM_I_VAL) {
        LOG_ERR("Invalid WHO_AM_I: 0x%02x, expected 0x%02x", who_am_i, LSM6DSO_WHO_AM_I_VAL);
        return -ENODEV;
    }
    LOG_INF("LSM6DSO WHO_AM_I check passed. ID: 0x%02x", who_am_i);

    // Set accelerometer ODR (12.5 Hz) and 2g range (0x20)
    ret = lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL1_XL, 0x20);
    if (ret != 0) {
        LOG_ERR("Failed to set CTRL1_XL register (err: %d)", ret);
        return ret;
    }

    // Set gyroscope ODR (12.5 Hz) and 250dps range (0x20)
    ret = lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL2_G, 0x20);
    if (ret != 0) {
        LOG_ERR("Failed to set CTRL2_G register (err: %d)", ret);
        return ret;
    }

    LOG_INF("LSM6DSO initialized successfully.");
    return 0;
}

/**
 * @brief Fetch raw accelerometer and gyroscope data from LSM6DSO sensor.
 * @param i2c_dev Pointer to I2C device structure.
 * @param raw_data_out Pointer to structure for storing raw data.
 * @return 0 on success, negative value on failure.
 */
static int lsm6dso_fetch_raw_data(const struct device *i2c_dev, struct lsm6dso_raw_data *raw_data_out)
{
    uint8_t accel_data[6];
    uint8_t gyro_data[6];
    int ret;

    // Read accelerometer data (6 bytes)
    ret = lsm6dso_i2c_reg_read_bytes(i2c_dev, LSM6DSO_REG_OUTX_L_XL, accel_data, 6);
    if (ret != 0) {
        LOG_ERR("Failed to read accelerometer data (err: %d).", ret);
        return ret;
    }
    // Raw data is 16-bit signed integer, low byte first
    raw_data_out->accel_x = (int16_t)(accel_data[0] | (accel_data[1] << 8));
    raw_data_out->accel_y = (int16_t)(accel_data[2] | (accel_data[3] << 8));
    raw_data_out->accel_z = (int16_t)(accel_data[4] | (accel_data[5] << 8));

    // Read gyroscope data (6 bytes)
    ret = lsm6dso_i2c_reg_read_bytes(i2c_dev, LSM6DSO_REG_OUTX_L_G, gyro_data, 6);
    if (ret != 0) {
        LOG_ERR("Failed to read gyroscope data (err: %d).", ret);
        return ret;
    }
    // Raw data is 16-bit signed integer, low byte first
    raw_data_out->gyro_x = (int16_t)(gyro_data[0] | (gyro_data[1] << 8));
    raw_data_out->gyro_y = (int16_t)(gyro_data[2] | (gyro_data[3] << 8));
    raw_data_out->gyro_z = (int16_t)(gyro_data[4] | (gyro_data[5] << 8));

    return 0;
}

/**
 * @brief Display raw accelerometer and gyroscope data.
 * @param raw_data Pointer to structure containing raw data.
 * @param count Polling counter.
 */
static void lsm6dso_display_raw_data(const struct lsm6dso_raw_data *raw_data, int count)
{
    printf("accel raw: X:%d Y:%d Z:%d (LSB)\n",
           raw_data->accel_x, raw_data->accel_y, raw_data->accel_z);
    printf("gyro raw: X:%d Y:%d Z:%d (LSB)\n",
           raw_data->gyro_x, raw_data->gyro_y, raw_data->gyro_z);
    printf("trig_cnt:%d\n\n", count);
}

// --- Main function ---

int main(void)
{
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c30));
    struct lsm6dso_raw_data sensor_data;
    static int trig_cnt = 0; // Ensure only initialized once in main scope

    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C device %s is not ready!", i2c_dev->name);
        return 0;
    }
    LOG_INF("I2C device %s is ready.", i2c_dev->name);

    if (lsm6dso_init(i2c_dev) != 0) {
        LOG_ERR("Failed to initialize LSM6DSO sensor.");
        return 0;
    }

    printf("Testing LSM6DSO sensor in polling mode (custom I2C driver) - Raw Data Output.\n\n");

    while (1) {
        trig_cnt++; // Increment counter at the start of each loop

        // Fetch raw data
        if (lsm6dso_fetch_raw_data(i2c_dev, &sensor_data) == 0) {
            // Display raw data
            lsm6dso_display_raw_data(&sensor_data, trig_cnt);
        } else {
            LOG_ERR("Failed to fetch data.");
        }

        k_sleep(K_MSEC(1000)); // Read once every second
    }

    return 0;
}