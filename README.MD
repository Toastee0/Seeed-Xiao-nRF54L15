# Seeed Studio XIAO nRF54L15 Zephyr Samples

This repository contains sample applications and simple example code for the [Seeed Studio XIAO nRF54L15](https://www.seeedstudio.com/XIAO-nRF54L15-p-6493.html) development board using Zephyr RTOS.

## Board Installation (Required First!)

Before building any samples, you must install the board definition files:

1. **Clone this repository** (with submodules):
   ```bash
   git clone --recursive https://github.com/Toastee0/Seeed-Xiao-nRF54L15.git
   ```

2. **Copy board files** to your nRF Connect SDK installation:
   ```bash
   # Copy the entire board directory
   cp -r platform-seeedboards/zephyr/boards/arm/xiao_nrf54l15 C:/ncs/v3.1.1/zephyr/boards/arm/
   
   # Or on Windows PowerShell:
   Copy-Item -Path "platform-seeedboards\zephyr\boards\arm\xiao_nrf54l15" -Destination "C:\ncs\v3.1.1\zephyr\boards\arm\" -Recurse
   ```

3. **Verify installation** - The board files should be at:
   ```
   C:\ncs\v3.1.1\zephyr\boards\arm\xiao_nrf54l15\
   ```

Now you can build any sample with `-b xiao_nrf54l15/nrf54l15/cpuapp`

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

#### ✅ Working Samples (Tested on Hardware):
- `adc/` - ADC variable blink sample - reads potentiometer on A0, controls LED blink rate (working - builds, flashes, and tested with pot)
- `bap_dmic/` - **BLE Audio wireless microphone** - Complete wireless microphone system using BLE Audio broadcast with LC3 codec and PDM microphone input (working - 16kHz real-time audio streaming, 99.9%+ packet success rate, tested with nRF52840 dongle receiver)
- `blink/` - Basic LED blinking sample (working - builds, flashes, and runs correctly)
- `button/` - Button input sample (working - builds, flashes, and runs correctly)
- `dmic/` - Digital Microphone sample with LED feedback and terminal bar graph (working - captures audio, displays levels, LED responds to volume thresholds)
- `dmic-recorder/` - Digital Microphone recorder with UART streaming - records 10-second audio clips to WAV file via Python script (working - 16kHz/16-bit PCM, button-triggered recording, 921600 baud transfer, tested and produces acceptable quality audio)
- `imu/` - LSM6DSO IMU sensor sample with custom I2C driver - reads raw accelerometer and gyroscope data (working - tested and verified with real sensor, outputs raw LSB values at 12.5Hz)
- `led_strip/` - LED strip control using WS2812/SK6812 (working - outputs on MOSI/D10 only, uses SPI backend)
- `pwm/` - PWM LED fade sample with smooth brightness control (working - tested on D1/P1.05, 1kHz PWM frequency)
- `pwm_rgb/` - RGB LED control with 3-channel PWM - rainbow effects, HSV color conversion, smooth color transitions (working - tested on D1/D2/D3, includes fade effects between colors)
- `ble_modern/` - Modern Bluetooth Low Energy beacon sample with LED status indication (working - advertises as beacon with URL, LED blinks to show running status, tested on hardware)

#### 🧪 Needs Testing (Board Config Ready):
- `battery/` - Battery monitoring sample (board config updated)
- `ble/` - Bluetooth Low Energy sample (has partial board config)
- `epaper/` - E-paper display sample (needs verification)
- `gps/` - GPS module sample (needs verification)
- `HPF-mspi/` - High Performance Framework MSPI sample (uses CPUFLPR core)
- `lowpower/` - Low power consumption sample (needs verification)
- `uart/` - UART communication sample (needs verification)
- `lpuart/` - Low Power UART communication sample (needs verification)

#### ⚠️ Known Limitations:
- **Hardware SPI00 Minimum Frequency**: The nRF54L15's hardware SPI00 peripheral (SPIM00) has a minimum frequency of 2MHz, which is too fast for reliable SD card initialization (requires 400kHz). Two solutions are provided:
  - `xiao_expanded/sd_card/` - Uses SPI bit-bang driver (software SPI)
  - `xiao_expanded/sd_card_hw/` - Uses hardware SPI20 which supports full frequency range (400kHz to 24MHz)

### BLE Audio System (`bap_*` samples):

#### Complete Wireless Microphone System
The `bap_dmic` and `bap_broadcast_sink` samples work together to create a complete BLE Audio wireless microphone:

- **`bap_dmic/`** (Transmitter - XIAO nRF54L15):
  - Captures audio from PDM20 microphone
  - Encodes with LC3 codec
  - Broadcasts via BLE Audio (BAP Broadcast Source)
  - Real-time RMS monitoring
  - 99.9%+ transmission success rate

- **`bap_broadcast_sink/`** (Receiver - nRF52840 Dongle):
  - Scans and connects to BLE Audio broadcasts
  - Decodes LC3 audio
  - Two output modes:
    - **Serial Console Mode**: Debug output showing reception statistics
    - **USB Audio Mode**: Direct playback through Windows speakers
  - Switch modes by editing `boards/nrf52840dongle_nrf52840.conf`

**Quick Start**: See `bap_dmic/README.md` for complete setup instructions including receiver configuration.

### Expansion Base Samples (`xiao_expanded/`):

#### ✅ Working Samples (Tested on Hardware):
- `xiao_expanded/attitude_monitor/` - IMU attitude monitor with OLED display (working - builds, flashes, and runs correctly)
- `xiao_expanded/audio_visualizer/` - Real-time audio visualizer with FFT and mel-spectrogram on OLED (working - captures DMIC audio, performs FFT, displays frequency bands)
- `xiao_expanded/buzzer/` - Button-triggered PWM buzzer control at 1kHz (working - tested with expansion base, press D1 button to sound buzzer on D3)
- `xiao_expanded/grove_relay/` - Expansion base Grove relay control sample (requires expansion base hardware & a grove relay connected on port a0/d0)
- `xiao_expanded/oled/` - OLED display sample for expansion base
- `xiao_expanded/sd_card/` - SD Card with expansion base using SPI bit-bang driver (working - uses software SPI to overcome SPI00/SPIM00 2MHz minimum frequency limitation, successfully reads/writes files)
- `xiao_expanded/sd_card_hw/` - SD Card with expansion base using hardware SPI20 peripheral (working - uses SDHC driver with hardware SPI20, supports 400kHz initialization and up to 24MHz data transfer, includes filesystem shell interface)

#### 🧪 Needs Testing (Board Config Ready):
- `xiao_expanded/rtc/` - Real-time clock sample for expansion base

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

