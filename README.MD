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
- `zephyr-HPF-mspi-ncs/` - High Performance Framework MSPI sample
- `zephyr-expansion-base-for-xiao/` - Samples for expansion base modules (buzzer, I2C SHT31, OLED, RTC)
- `old/` - Older versions of samples (PlatformIO based, not supported)

Each sample directory contains:
- `CMakeLists.txt` - Build configuration for nRF Connect SDK
- `prj.conf` - Project configuration
- `src/` - Source code
- `README.md` - Sample-specific documentation
- `sample.yaml` - Sample metadata to allow it to appear in nRF Connect for VS Code


## PlatformIO Support

PlatformIO is **not supported** for this repository. The Zephyr implementation in PlatformIO is out of date as of 9/17/2025 and incompatible with the current samples.

## Getting Started

### Prerequisites
- nRF Connect for VS Code
- nRF Connect SDK (for NCS samples)
- Seeed Xiao nRF54L15 or xiao nRF54l15 Sense
- These samples should also work on the nRF54l15-DK board (todo: add nRF54l15-DK board to the sample.yaml files)

### Building and Running
install nRF Connect for VS Code : https://www.youtube.com/watch?v=EAJdOqsL9m8  
This video covers installation of the required tools to support the extension, nRF toolchain and nRF connect sdk.
The samples in this repository have been tested with nRF Connect for VS Code version 3.1.0.
Opening the sample folder in vs code will automatically add the sample as an application in nrf connect for vs code.

Add an application and build configuration for the xiao nRF54L15 board to the nRF Connect for VS Code Workspace.

## Resources

- [Seeed Xiao nRF54L15 Wiki](https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/)
- [Purchase Link](https://www.seeedstudio.com/XIAO-nRF54L15-Sense-p-6494.html)
- [Zephyr Documentation](https://docs.zephyrproject.org/)

## Contributing

Contributions are welcome! Please submit issues and pull requests.

## License

This project is licensed under the Apache 2.0 License.
