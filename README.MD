# Seeed Xiao nRF54L15 Zephyr Samples

This repository contains sample applications for the Seeed Studio Xiao nRF54L15 development board using the Zephyr RTOS.

## Repository Structure

The repository is organized into the following sample projects:

- `zephyr-adc-ncs/` - Analog-to-Digital Converter sample using nRF Connect SDK
- `zephyr-battery-ncs/` - Battery monitoring sample
- `zephyr-ble-ncs/` - Bluetooth Low Energy sample
- `zephyr-blink-ncs/` - Basic LED blinking sample
- `zephyr-button-ncs/` - Button input sample
- `zephyr-dmic-ncs/` - Digital Microphone sample
- `zephyr-dmic-recorder-ncs/` - Digital Microphone recorder sample
- `zephyr-epaper-ncs/` - E-paper display sample
- `zephyr-gpio-ncs/` - GPIO control sample
- `zephyr-gps-ncs/` - GPS module sample
- `zephyr-imu-ncs/` - Inertial Measurement Unit (IMU) sample
- `zephyr-lowpower-ncs/` - Low power consumption sample
- `zephyr-pwm-ncs/` - Pulse Width Modulation sample
- `zephyr-HPF-mspi-ncs/` - High Performance Frequency MSPI sample
- `zephyr-expansion-base-for-xiao/` - Samples for expansion base modules (buzzer, I2C SHT31, OLED, RTC)
- `old/` - Older versions of samples (PlatformIO based)

Each sample directory contains:
- `CMakeLists.txt` - Build configuration for nRF Connect SDK
- `prj.conf` - Project configuration
- `src/` - Source code
- `README.md` - Sample-specific documentation
- `sample.yaml` - Sample metadata

Some samples also include PlatformIO configurations in the `old/` folder.

## Getting Started

### Prerequisites

- nRF Connect SDK (for NCS samples)
- PlatformIO (for PlatformIO samples)
- Seeed Xiao nRF54L15 board

### Building and Running

For NCS samples:
1. Navigate to the sample directory
2. Use nRF Connect for VS Code or command line to build and flash

For PlatformIO samples (in `old/`):
1. Open the project in PlatformIO
2. Build and upload

## Resources

- [Seeed Xiao nRF54L15 Wiki](https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/)
- [Purchase Link](https://www.seeedstudio.com/XIAO-nRF54L15-Sense-p-6494.html)
- [Zephyr Documentation](https://docs.zephyrproject.org/)

## Contributing

Contributions are welcome! Please submit issues and pull requests.

## License

This project is licensed under the Apache 2.0 License.
