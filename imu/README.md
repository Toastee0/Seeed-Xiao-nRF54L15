# IMU LSM6DSO Sensor Sample

This sample demonstrates I2C communication with the LSM6DSO IMU sensor to read accelerometer and gyroscope data on the Seeed XIAO nRF54L15.

## Features

- I2C communication with LSM6DSO sensor
- Device identification and initialization
- Real-time accelerometer and gyroscope data reading
- Raw data output in LSB units
- Configurable sensor ODR and range settings
- Comprehensive error handling and logging

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- LSM6DSO IMU sensor connected via I2C
- I2C connection to i2c30 device node

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The sample will continuously read and display IMU data every second:

```
Testing LSM6DSO sensor in polling mode (custom I2C driver) - Raw Data Output.

accel raw: X:156 Y:-89 Z:16234 (LSB)
gyro raw: X:23 Y:-45 Z:12 (LSB)
trig_cnt:1

accel raw: X:145 Y:-92 Z:16198 (LSB)
gyro raw: X:18 Y:-38 Z:15 (LSB)
trig_cnt:2
```

## Configuration

- I2C and GPIO drivers are enabled
- Sensor subsystem is enabled
- Console and UART are enabled for output
- Newlib libc with float printf support
- LSM6DSO configured for:
  - Accelerometer: 12.5 Hz ODR, ±2g range
  - Gyroscope: 12.5 Hz ODR, ±250 dps range

## Technical Details

- I2C Address: 0x6A
- WHO_AM_I verification: 0x6A
- 16-bit signed raw data output
- Polling mode with 1-second intervals
- Custom I2C driver implementation for direct register access