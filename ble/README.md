# BLE LED Control Sample for XIAO nRF54L15

This sample demonstrates a Bluetooth Low Energy (BLE) peripheral that allows remote control of the onboard LED via a custom GATT service.

## Overview

The application creates a BLE peripheral device that:
- Advertises as "XIAO-nRF54L15-BLE"
- Provides a custom GATT service for LED control
- Controls the onboard LED (GPIO P2.01) based on BLE commands
- Supports both read and write operations

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- USB cable for power and programming
- BLE central device (smartphone, tablet, or computer with BLE support)

## BLE Service Details

### Custom GATT Service
- **Service UUID**: `8e7f1a23-4b2c-11ee-be56-0242ac120002`

### Characteristics
1. **LED Control (Write)**
   - **UUID**: `8e7f1a24-4b2c-11ee-be56-0242ac120002`
   - **Properties**: Write
   - **Description**: Write 0x00 to turn LED off, 0x01 to turn LED on

2. **LED Status (Read)**
   - **UUID**: `8e7f1a25-4b2c-11ee-be56-0242ac120003`
   - **Properties**: Read
   - **Description**: Read current LED state (0x00 = off, 0x01 = on)

## Building and Running

### Prerequisites
- nRF Connect SDK v3.1.1 or later
- nRF Connect for VS Code extension
- XIAO nRF54L15 board support

### Build Instructions
1. Open the project in nRF Connect for VS Code
2. Select board: `xiao_nrf54l15/nrf54l15/cpuapp`
3. Build the project
4. Flash to the XIAO nRF54L15

### Expected Behavior
1. After flashing, the device will start advertising
2. The serial console will show startup messages
3. Connect with a BLE scanner app to control the LED

## Testing with BLE Apps

### Recommended Apps
- **nRF Connect** (Android/iOS) - Nordic Semiconductor
- **BLE Scanner** (Android/iOS)
- **LightBlue** (iOS/macOS)

### Connection Steps
1. Open your BLE scanner app
2. Look for device named "XIAO-nRF54L15-BLE"
3. Connect to the device
4. Find the custom service (UUID: 8e7f1a23...)
5. Write to the control characteristic:
   - Write `0x00` to turn LED off
   - Write `0x01` to turn LED on
6. Read the status characteristic to verify LED state

## Hardware Configuration

### LED
- **GPIO**: P2.01 (GPIO2, pin 1)
- **Active**: High
- **Function**: Visual feedback for BLE commands

### Button (Optional)
- **GPIO**: P1.10 (GPIO1, pin 10)  
- **Active**: Low (with pull-up)
- **Function**: User input (available for future enhancements)

## Power Management

The sample includes power management features:
- Low power mode when not connected
- Optimized for battery operation
- Can be disabled for debugging by modifying board configuration

## Troubleshooting

### Common Issues
1. **Device not advertising**
   - Check Bluetooth is enabled in configuration
   - Verify GPIO configuration in overlay
   - Check console output for error messages

2. **Cannot connect**
   - Ensure device is in range
   - Try resetting the device
   - Check if already connected to another central

3. **LED not responding**
   - Verify GPIO configuration
   - Check device tree overlay
   - Monitor console output for write operations

### Console Output
Connect a serial terminal (115200 baud) to see:
- Startup messages
- Connection status
- LED control commands

## Extending the Sample

This sample can be extended to:
- Add button input for local LED control
- Implement LED patterns or PWM control
- Add additional characteristics (temperature, battery level, etc.)
- Support multiple connections
- Add notification/indication support

## Files Structure

```
ble/
├── boards/
│   ├── xiao_nrf54l15_nrf54l15_cpuapp.conf    # Board-specific configuration
│   └── xiao_nrf54l15_nrf54l15_cpuapp.overlay # Device tree overlay
├── src/
│   └── main.c                                # Main application
├── CMakeLists.txt                           # Build configuration
├── prj.conf                                 # Project configuration
├── sample.yaml                              # Sample metadata
└── README.md                                # This file
```

## Configuration Options

Key configuration options in `prj.conf`:
- `CONFIG_BT=y` - Enable Bluetooth
- `CONFIG_BT_PERIPHERAL=y` - Enable peripheral role
- `CONFIG_BT_DEVICE_NAME` - Device name for advertising
- `CONFIG_GPIO=y` - Enable GPIO support

Board-specific options in `boards/xiao_nrf54l15_nrf54l15_cpuapp.conf`:
- Console and logging settings
- Power management options
- Memory optimization settings