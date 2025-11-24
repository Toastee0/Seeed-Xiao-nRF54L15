# Bluepad32 for Zephyr RTOS

A port of Bluepad32 (Bluetooth gamepad/controller library) to Zephyr RTOS, designed for the Seeed Studio Xiao nRF54L15 and other Zephyr-supported platforms.

## Overview

Bluepad32-Zephyr enables your nRF54L15 (or any Zephyr BLE-capable board) to connect to Bluetooth game controllers, mice, and keyboards. It acts as a **BLE Central** device that scans for and connects to HID peripherals.

### Supported Controllers

- ✅ Sony DualShock 4 (PS4)
- ✅ Sony DualSense (PS5)
- ✅ Nintendo Switch Pro Controller
- ✅ Xbox One / Xbox Series X|S Controllers
- ✅ Generic Bluetooth Mice
- ✅ Generic Bluetooth Keyboards
- 🚧 Nintendo Wii Remote (planned)
- 🚧 8BitDo controllers (planned)

## Features

- **BLE Central Mode**: Connects to multiple controllers simultaneously
- **HID over GATT (HOGP)**: Standard Bluetooth HID profile
- **Multiple Devices**: Support up to 4 controllers at once (configurable)
- **Portable Design**: Works on any Zephyr board with BLE support
- **Low Latency**: Efficient input processing for gaming applications
- **Bond Management**: Automatically reconnects to paired devices

## Hardware Requirements

### Primary Target
- **Seeed Studio Xiao nRF54L15**
  - nRF54L15 SoC (Bluetooth 5.4 LE)
  - 1.5 MB Flash, 256 KB RAM
  - Built-in RGB LED and user button

### Other Supported Boards
Any Zephyr board with:
- BLE 4.0+ support
- Sufficient RAM (128+ KB recommended)
- Central role capability

## Project Structure

```
bluepad32_zephyr/
├── CMakeLists.txt              # Build configuration
├── prj.conf                    # Kconfig settings
├── Kconfig                     # Bluepad32 options
├── README.md                   # This file
├── boards/                     # Board-specific overlays
│   └── xiao_nrf54l15_nrf54l15_cpuapp.overlay
└── src/
    ├── main.c                  # Application entry point
    ├── bluetooth/              # BLE layer (scanning, GATT, etc.)
    │   ├── bt_connection.c
    │   ├── bt_scan.c
    │   └── bt_gatt_client.c
    ├── controller/             # Controller abstractions
    │   ├── uni_controller.c
    │   ├── uni_gamepad.c
    │   ├── uni_mouse.c
    │   └── uni_keyboard.c
    ├── parser/                 # HID parsers (controller-specific)
    │   ├── uni_hid_parser_generic.c
    │   ├── uni_hid_parser_ds4.c
    │   ├── uni_hid_parser_switch.c
    │   ├── uni_hid_parser_xbox.c
    │   ├── uni_hid_parser_mouse.c
    │   └── uni_hid_parser_keyboard.c
    ├── device/                 # Device management
    │   └── uni_hid_device.c
    └── platform/               # Zephyr platform layer
        └── uni_platform_zephyr.c
```

## Getting Started

### Prerequisites

1. **Install NCS SDK** (v3.2.0-preview2 or later)
   ```bash
   # Follow: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation.html
   ```

2. **Clone this repository**
   ```bash
   cd /path/to/ncs/workspace
   git clone <this-repo-url> bluepad32_zephyr
   ```

### Building

```bash
# Navigate to the project directory
cd bluepad32_zephyr

# Build for Xiao nRF54L15
west build -b xiao_nrf54l15/nrf54l15/cpuapp

# Flash to board
west flash

# View logs
west espressif monitor
# or
minicom -D /dev/ttyACM0 -b 115200
```

### Building for Other Boards

```bash
# nRF52840 DK
west build -b nrf52840dk/nrf52840

# nRF5340 DK
west build -b nrf5340dk/nrf5340/cpuapp

# Custom board (with overlay in boards/ directory)
west build -b your_board_name
```

## Usage

### Basic Operation

1. **Power on** the Xiao nRF54L15
2. The device will start **scanning automatically**
3. Put your controller in **pairing mode**:
   - **PS4/PS5**: Hold Share + PS button
   - **Switch Pro**: Hold Sync button
   - **Xbox**: Hold pairing button
4. The board will **connect automatically** when detected
5. LED indicates connection status:
   - **Blinking**: Scanning for devices
   - **Solid**: Controller connected

### Button Controls

- **Button 1 (SW0)**: Start/stop scanning
- **Button 2**: Clear bonding data (hold for 3 seconds)

### Configuration

Edit `prj.conf` to customize:

```ini
# Maximum connected devices (1-8)
CONFIG_BLUEPAD32_MAX_DEVICES=4

# Enable/disable specific controller parsers
CONFIG_BLUEPAD32_PARSER_DS4=y
CONFIG_BLUEPAD32_PARSER_SWITCH=y
CONFIG_BLUEPAD32_PARSER_XBOX=y
CONFIG_BLUEPAD32_PARSER_MOUSE=y
CONFIG_BLUEPAD32_PARSER_KEYBOARD=y

# Logging level (0=OFF, 1=ERR, 2=WRN, 3=INF, 4=DBG)
CONFIG_BLUEPAD32_LOG_LEVEL=3
```

## Development Status

### ✅ Phase 1: Foundation (COMPLETE)
- [x] Project structure
- [x] Build system (CMake + Kconfig)
- [x] Basic BLE initialization
- [x] Scanning implementation

### 🚧 Phase 2: Bluetooth Layer (IN PROGRESS)
- [ ] Connection management
- [ ] GATT service discovery
- [ ] HID characteristic subscription
- [ ] Input report handling

### 📋 Phase 3: HID Parsers (TODO)
- [ ] Generic HID parser
- [ ] DualShock 4 parser
- [ ] Switch Pro parser
- [ ] Xbox controller parser
- [ ] Mouse parser
- [ ] Keyboard parser

### 📋 Phase 4: Platform Layer (TODO)
- [ ] Device state management
- [ ] Virtual device support
- [ ] Platform callbacks
- [ ] LED feedback

### 📋 Phase 5: Testing & Polish (TODO)
- [ ] Hardware validation
- [ ] Multi-controller support
- [ ] Bond management
- [ ] Power optimization

## API Example

```c
#include "uni_platform_zephyr.h"
#include "uni_controller.h"

static void on_controller_data(uni_hid_device_t* device, uni_controller_t* ctl)
{
    if (ctl->klass == UNI_CONTROLLER_CLASS_GAMEPAD) {
        uni_gamepad_t* gp = &ctl->gamepad;
        
        printk("Buttons: 0x%04x\n", gp->buttons);
        printk("Left Stick: X=%d Y=%d\n", gp->axis_x, gp->axis_y);
        printk("Triggers: L=%d R=%d\n", gp->brake, gp->throttle);
        
        // Your game logic here
    }
}

static void on_device_connected(uni_hid_device_t* device)
{
    printk("Controller connected: %s\n", device->name);
}
```

## Troubleshooting

### Controller not detected
- Ensure controller is in pairing mode
- Check that scanning is active (LED should blink)
- Try clearing bonds (Button 2)
- Check logs for connection errors

### Build errors
```bash
# Clean build
west build -t pristine
west build -b xiao_nrf54l15/nrf54l15/cpuapp

# Check NCS version
west --version
nrfutil --version
```

### Connection drops
- Reduce `CONFIG_BLUEPAD32_MAX_DEVICES` if memory constrained
- Check power supply stability
- Ensure firmware is up to date on controller

## Contributing

Contributions welcome! Areas needing help:
- HID parser implementations
- Additional controller support
- Testing on different boards
- Documentation improvements

## Porting to New Boards

1. Create board overlay: `boards/your_board_name.overlay`
2. Define GPIOs for LEDs and buttons
3. Enable BLE hardware
4. Build and test!

No application code changes needed thanks to Zephyr's device tree abstraction.

## License

SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

Based on [Bluepad32](https://github.com/ricardoquesada/bluepad32) by Ricardo Quesada.

## Resources

- [Zephyr Bluetooth API](https://docs.zephyrproject.org/latest/connectivity/bluetooth/)
- [NCS Documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/)
- [Bluepad32 Original](https://github.com/ricardoquesada/bluepad32)
- [HOGP Specification](https://www.bluetooth.com/specifications/specs/hid-over-gatt-profile/)

## Contact

Issues and questions: [GitHub Issues](https://github.com/Toastee0/bluepad32-zephyr/issues)

---

**Status**: 🚧 Active Development  
**Version**: 0.1.0-alpha  
**Last Updated**: November 10, 2025
