# Seeed Xiao nRF54L15 Zephyr Samples

This repository contains sample applications and simple example code for the Seeed Studio Xiao nRF54L15 development board using Zephyr RTOS.

## Centralized Libraries

All external libraries are managed as git submodules in the `lib/` directory to avoid duplication across samples:

- **lib/u8g2/** - U8g2 graphics library for OLED displays
- **lib/MadgwickAHRS/** - Arduino Madgwick AHRS filter for IMU sensor fusion

### Using Libraries in Samples

When creating or building samples that use these libraries:

1. **Initialize submodules** (first time):
   ```bash
   git submodule update --init --recursive
   ```

2. **Reference libraries** in CMakeLists.txt:
   ```cmake
   # Example: Include U8g2 library
   file(GLOB U8G2_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/u8g2/csrc/*.c)
   target_sources(app PRIVATE ${U8G2_SOURCES})
   target_include_directories(app PRIVATE ../../../lib/u8g2/csrc)
   ```

3. **Include headers** in source code:
   ```cpp
   #include "../../../lib/u8g2/csrc/u8g2.h"
   #include "../../../lib/MadgwickAHRS/src/MadgwickAHRS.h"
   ```

## Repository Structure

The repository is organized into the following sample projects:

### Status Legend:
- ✅ **Working** - Successfully built, flashed, and tested on XIAO nRF54L15 hardware
- ⚠️ **Needs Fix** - Build fails or requires configuration updates  
- 🧪 **Testing** - Board config ready, needs build and hardware testing
- ❓ **Untested** - Not yet tested or missing board configuration

### Sample Projects:

- ✅ `adc/` - ADC variable blink sample - reads potentiometer on A0, controls LED blink rate (working - builds, flashes, and tested with pot)
- 🧪 `battery/` - Battery monitoring sample (board config updated)
- 🧪 `ble/` - Bluetooth Low Energy sample (has partial board config)
- ✅ `blink/` - Basic LED blinking sample (working - builds, flashes, and runs correctly)
- ✅ `button/` - Button input sample (working - builds, flashes, and runs correctly)
- 🧪 `dmic/` - Digital Microphone sample (needs verification)
- 🧪 `dmic-recorder/` - Digital Microphone recorder sample (needs verification)
- 🧪 `epaper/` - E-paper display sample (needs verification)
- 🧪 `gps/` - GPS module sample (needs verification)
- 🧪 `imu/` - Inertial Measurement Unit sample (board config added - ready for testing)
- ✅ `led_strip/` - LED strip control using WS2812/SK6812 (working - outputs on MOSI/D10 only, uses SPI backend)
- 🧪 `lowpower/` - Low power consumption sample (needs verification)
- 🧪 `pwm/` - Pulse Width Modulation sample (needs verification)
- 🧪 `HPF-mspi/` - High Performance Framework MSPI sample (uses CPUFLPR core)
- ❌ `sdcard/` - SD Card sample (not working - moved to Not_working/)
- 🧪 `uart/` - UART communication sample (needs verification)
- 🧪 `lpuart/` - Low Power UART communication sample (needs verification)

### Expansion Base Samples (`xiao_expanded/`):
- ✅ `xiao_expanded/attitude_monitor/` - IMU attitude monitor with OLED display (working - builds, flashes, and runs correctly)
- 🧪 `xiao_expanded/sd_card/` - SD Card with expansion base (board config added)
- 🧪 `xiao_expanded/buzzer/` - Buzzer control sample for expansion base
- ✅ `xiao_expanded/grove_relay/` - Expansion base Grove relay control sample (requires expansion base hardware & a grove relay connected on port a0/d0)
- 🧪 `xiao_expanded/i2c-sht31/` - I2C SHT31 temperature/humidity sensor sample
- 🧪 `xiao_expanded/oled/` - OLED display sample for expansion base
- 🧪 `xiao_expanded/rtc/` - Real-time clock sample for expansion base

### Applications (`applications/`):

Complete applications that combine multiple features for specific use cases:

- ✅ `applications/blink_uart/` - LED blink with UART output and system-off power management (working - demonstrates power management and low-power modes)

Applications differ from samples in that they:
- Combine multiple peripheral features
- Implement complete end-to-end functionality
- May include power management and sleep modes
- Are designed for specific real-world use cases

Each sample/application directory contains:
- `CMakeLists.txt` - Build configuration for nRF Connect SDK
- `prj.conf` - Project configuration
- `src/` - Source code
- `README.md` - Sample-specific documentation
- `sample.yaml` - Sample metadata that makes the board appear in the compatible section of nRF Connect's build UI, enabling automatic selection of the correct `xiao_nrf54l15/nrf54l15/cpuapp` profile
- `boards/` - **Required** board-specific configuration folder containing:
  - `xiao_nrf54l15_nrf54l15_cpuapp.conf` - Board-specific Kconfig options
  - `xiao_nrf54l15_nrf54l15_cpuapp.overlay` - Board-specific devicetree overlay

**Important**: All samples and applications must include:
1. A `sample.yaml` file listing the board as compatible
2. A `boards/` folder with the Xiao nRF54L15 configuration files

These files work together to ensure that when you add the application in nRF Connect for VS Code, the board appears as compatible and the correct board-specific options and devicetree configurations are automatically applied.


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
If you want to support the development of more samples for this board, please consider purchasing through the affiliate link below:
- [AffiliatePurchase Link](https://www.seeedstudio.com/XIAO-nRF54L15-Sense-p-6494.html?sensecap_affiliate=JG7nwfh&referring_service=link)
- [Zephyr Documentation](https://docs.zephyrproject.org/)

## Contributing

Contributions are welcome! Please submit issues and pull requests.

## License

This project is licensed under the Apache 2.0 License.

other resources
https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/samples.html

