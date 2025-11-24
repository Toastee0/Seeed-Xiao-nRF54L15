# Copilot Instructions for bluepad32_zephyr Project

## Project Overview
Porting Bluepad32 (game controller library) from ESP-IDF/BTstack to Zephyr RTOS on Nordic nRF54L15.

## Build Environment

**CRITICAL**: Always set these environment variables before building:

```powershell
$env:ZEPHYR_BASE = "C:\ncs\v3.2.0-preview2\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\c1a76fddb2\opt\zephyr-sdk"
```

## Standard Build Commands

### Build (from project root)
```powershell
$env:ZEPHYR_BASE = "C:\ncs\v3.2.0-preview2\zephyr"; $env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"; $env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\c1a76fddb2\opt\zephyr-sdk"; west build --build-dir m:/nRF54L15/bluepad32_zephyr/build m:/nRF54L15/bluepad32_zephyr --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
```

### Flash
```powershell
west flash --build-dir m:/nRF54L15/bluepad32_zephyr/build
```

### Clean Build (pristine)
```powershell
$env:ZEPHYR_BASE = "C:\ncs\v3.2.0-preview2\zephyr"; $env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"; $env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\c1a76fddb2\opt\zephyr-sdk"; west build --build-dir m:/nRF54L15/bluepad32_zephyr/build m:/nRF54L15/bluepad32_zephyr --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
```

## Project Structure

- `src/main.c` - Main application, BLE scanning, connection management
- `src/bluetooth/bt_gatt_client.c` - GATT client for HID service discovery
- `prj.conf` - Zephyr Kconfig settings (BT Central, GATT, SMP)
- `CMakeLists.txt` - Build configuration

## Hardware

- **Board**: Seeed Studio Xiao nRF54L15 (nrf54l15/cpuapp)
- **Controller**: Xbox Wireless Controller (F4:6A:D7:64:1F:71)
- **BT Stack**: Nordic SoftDevice Controller (not ESP-IDF Bluedroid)

## Key Differences from Original Bluepad32

1. **No manual pairing trigger** - Zephyr handles security elevation automatically during GATT operations
2. **Connection flow**: Connection → GATT discovery starts immediately → Pairing happens automatically when needed
3. **UUID definitions**: Use Zephyr's built-in `BT_UUID_*` constants, don't redeclare with `BT_UUID_DECLARE_16()`

## Current Status

- ✅ BLE scanning and filtering
- ✅ Connection establishment
- ✅ Automatic pairing (triggered by GATT operations)
- ✅ HID Service discovery (UUID 0x1812)
- ✅ Report Map reading (283 bytes)
- 🔄 CCC descriptor discovery (handle 31, UUID 0x2902)
- ⏳ Notification subscription
- ⏳ HID report parsing

## Debugging

Serial console: 115200 8N1
Monitor with: VS Code Serial Monitor or PuTTY

## Reference Files

- `co-pilot_west_workflow.md` - Detailed west build commands and recovery procedures
- `agents_bluepad32_porting.md` - Porting notes and progress tracking
