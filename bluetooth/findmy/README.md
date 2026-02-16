# FindMy Beacon for nRF54L15

Port of macless-haystack firmware to nRF54L15 using nRF Connect SDK.

## Overview

This implements Apple's FindMy protocol, allowing the nRF54L15 to be tracked via Apple's Find My network without requiring an actual AirTag.

Based on:
- [macless-haystack](https://github.com/dchristl/macless-haystack) nRF52 firmware
- [OpenHaystack](https://github.com/seemoo-lab/openhaystack) protocol

## Features

- ✅ Non-connectable BLE advertisements (FindMy protocol)
- ✅ Multiple key support with 30-minute rotation
- ✅ Battery status monitoring (ADC integration TODO)
- ✅ Zephyr RTOS power management
- ✅ Compatible with macless-haystack backend

## Hardware Requirements

- Seeed Studio XIAO nRF54L15
- OR nRF54L15 DK
- OR any nRF54L15-based board

## Software Requirements

- nRF Connect SDK v2.7.0+ (v3.2.0-preview2 recommended)
- Zephyr SDK
- west tool

## Build Instructions

### 1. Generate Keys

First, generate your FindMy keys using the macless-haystack tool:

```bash
cd M:\nrf54L15_projects\macless-haystack
python generate_keys.py -p MYDEVICE
```

This creates `output/MYDEVICE_keyfile` with your advertisement keys.

### 2. Update Firmware with Your Key

Edit `src/main.c` and replace the placeholder key:

```c
static char public_keys[MAX_KEYS][28] = {
    "OFFLINEFINDINGPUBLICKEYHERE!",  // Replace with your actual key
};
```

You can add multiple keys for rotation (up to 20):

```c
static char public_keys[MAX_KEYS][28] = {
    "YOUR_FIRST_KEY_HERE_28BYTES",
    "YOUR_SECOND_KEY_HERE_28BYTE",
    "YOUR_THIRD_KEY_HERE_28BYTES",
};
```

### 3. Build

```powershell
cd M:\nrf54L15_projects\Seeed-Xiao-nRF54L15\bluetooth\findmy

# Build for XIAO nRF54L15
west build --build-dir build --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild

# OR for nRF54L15 DK
west build --build-dir build --pristine --board nrf54l15dk/nrf54l15/cpuapp --sysbuild
```

### 4. Flash

```powershell
west flash --build-dir build
```

### 5. Verify

The device should start advertising immediately. You can verify with:
- nRF Connect mobile app (look for manufacturer data 0x4C00)
- macless-haystack web interface

## Configuration

### Advertisement Interval

Default: 5 seconds (5000ms)

Edit in `src/main.c`:
```c
#define ADVERTISING_INTERVAL_MS     5000
```

### Key Rotation Interval

Default: 30 minutes

Edit in `src/main.c`:
```c
#define KEY_ROTATION_INTERVAL_MIN   30
```

### Battery Update Interval

Default: 14 days

Edit in `src/main.c`:
```c
#define BATTERY_UPDATE_INTERVAL_DAYS 14
```

### Logging

Disable logging for production (reduces power consumption):

In `prj.conf`, set:
```
CONFIG_LOG=n
CONFIG_PRINTK=n
```

## Power Consumption

Expected current draw:
- **Active advertising**: ~5mA
- **Sleep between advertisements**: <100µA
- **Average** (with 5s interval): ~200-300µA

Target: Months on CR2032 coin cell (not yet optimized)

## macless-haystack Backend Integration

1. Set up macless-haystack backend (see main repo)
2. Import your `MYDEVICE_devices.json` to the frontend
3. The beacon will appear on the map as it's detected by nearby Apple devices

## Troubleshooting

### Device not advertising

Check logs:
```powershell
west build --build-dir build --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
cd build && west debug
```

### Not visible in FindMy network

- Ensure key is correctly formatted (28 bytes)
- Verify advertisement packet with nRF Connect app
- Check that macless-haystack backend is running

### Build errors

Ensure environment variables are set:
```powershell
$env:ZEPHYR_BASE
$env:ZEPHYR_TOOLCHAIN_VARIANT
$env:ZEPHYR_SDK_INSTALL_DIR
```

### Recovery (bricked device)

```powershell
python -m pyocd erase -t nrf54l --mass -v
python -m pyocd erase -t nrf54l --mass -v  # Run twice
```

## TODO

- [ ] Integrate ADC for actual battery monitoring
- [ ] Deep sleep optimization (target <50µA average)
- [ ] NVS integration for key storage (no recompile needed)
- [ ] OTA firmware updates
- [ ] BLE Channel Sounding integration (nRF54L15 feature)

## Protocol Details

### Advertisement Packet Format

```
Byte 0:     0x1E              Length (30 bytes)
Byte 1:     0xFF              Manufacturer Specific Data
Byte 2-3:   0x4C 0x00         Apple Company ID
Byte 4:     0x12              Offline Finding type
Byte 5:     0x19              Length (25 bytes)
Byte 6:     0x00              State (battery status)
Byte 7-28:  <22 bytes>        Public key bytes 6-27
Byte 29:    <2 bits>          First 2 bits of key byte 0
Byte 30:    0x00              Hint
```

### MAC Address Derivation

```c
MAC[5] = key[0] | 0xC0  // Mark as random static
MAC[4] = key[1]
MAC[3] = key[2]
MAC[2] = key[3]
MAC[1] = key[4]
MAC[0] = key[5]
```

## License

AGPL-3.0 (same as macless-haystack)

## Credits

- [macless-haystack](https://github.com/dchristl/macless-haystack) by dchristl
- [OpenHaystack](https://github.com/seemoo-lab/openhaystack) by Secure Mobile Networking Lab
- nRF52 firmware by acalatrava
