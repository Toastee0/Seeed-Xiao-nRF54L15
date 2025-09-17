# Expansion Board RTC (PCF8563) Sample

This sample demonstrates how to use the PCF8563 real-time clock (RTC) chip with the Seeed XIAO nRF54L15 and expansion board.

## Overview

The sample shows how to:
- Initialize the PCF8563 RTC chip via I2C
- Set and read time from the RTC
- Display current time continuously
- Handle time synchronization and formatting

## Requirements

### Hardware
- Seeed XIAO nRF54L15 development board
- Seeed XIAO Expansion Board with PCF8563 RTC chip
- USB-C cable for power and programming

### Software
- nRF Connect SDK (NCS) v2.8.0 or later
- West build tool
- nRF Command Line Tools

## Building and Running

### Using west

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

### Using CMake directly

```bash
cmake -Bbuild -GNinja -DBOARD=xiao_nrf54l15/nrf54l15/cpuapp
ninja -Cbuild
```

## Expected Output

```
*** RTC PCF8563 Sample ***
RTC initialized successfully
Current time: 2024-01-15 14:30:25
Current time: 2024-01-15 14:30:26
Current time: 2024-01-15 14:30:27
...
```

## Key Features

### RTC Functionality
- Time setting and reading
- I2C communication with PCF8563
- Time formatting and display
- Continuous time updates

### Hardware Integration
- I2C interface configuration
- Expansion board PCF8563 support
- Real-time clock synchronization
- System time management

## Code Structure

- **main.c**: Main application with RTC initialization and time display
- **prj.conf**: Project configuration with I2C and RTC drivers
- **CMakeLists.txt**: Build configuration
- **boards/**: Board-specific device tree overlays

## Device Tree Configuration

The sample uses custom device tree overlay to configure:
- I2C interface pins (SDA: P1.04, SCL: P1.05)  
- PCF8563 RTC chip address (0x51)
- Pull-up resistor configuration
- Clock frequency settings

## Troubleshooting

### Common Issues

1. **RTC not detected**
   - Check I2C connections
   - Verify expansion board is properly connected
   - Ensure correct I2C address (0x51)

2. **Time not updating**
   - Check RTC initialization
   - Verify I2C communication
   - Check device tree configuration

3. **Build errors**
   - Ensure NCS environment is properly set up
   - Check board target is correct
   - Verify all dependencies are available

## References

- [PCF8563 Datasheet](https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf)
- [Zephyr I2C API](https://docs.zephyrproject.org/latest/hardware/peripherals/i2c.html)
- [nRF Connect SDK Documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/index.html)
- [Seeed XIAO nRF54L15 Wiki](https://wiki.seeedstudio.com/xiao_nrf54l15/)