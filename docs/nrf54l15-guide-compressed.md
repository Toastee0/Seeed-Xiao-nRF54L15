#workflow_compressed for reduced token usage
# nRF54L15 Development Guide

Guide for Seeed Studio Xiao nRF54L15 with nRF Connect SDK v2.7.0+ (v3.2.0-preview2+ recommended).

## Environment Setup

### Required Variables
```powershell
$env:ZEPHYR_BASE = "C:\ncs\<VERSION>\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"
```

**Find paths:**
```powershell
Get-ChildItem C:\ncs\ | Where-Object {$_.Name -match '^v\d'}  # SDK versions
Get-ChildItem C:\ncs\toolchains\ | Where-Object {$_.Name -match '^[a-f0-9]+$'}  # Toolchain hash
```

### Permanent Setup (Choose One)
**GUI:** Win+R → `sysdm.cpl` → Advanced → Environment Variables  
**PowerShell:** `[Environment]::SetEnvironmentVariable("ZEPHYR_BASE", "C:\ncs\<VERSION>\zephyr", "User")`  
**CMD:** `setx ZEPHYR_BASE "C:\ncs\<VERSION>\zephyr"`

*Restart terminal after setting. Repeat for all three variables.*

## Quick Reference

### Build & Flash
```powershell
# Application core (ARM Cortex-M33)
west build --build-dir <sample>/build <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
west flash --build-dir <sample>/build

# FLPR core (RISC-V)
west build --build-dir <sample>/build_flpr <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuflpr --sysbuild
west flash --build-dir <sample>/build_flpr
```

### Multi-Core Build
**Sysbuild (recommended):** Builds both cores automatically if configured with `sysbuild.conf`/`Kconfig.sysbuild`/`sysbuild.cmake`.

**Separate images:**
```powershell
west build -p -b xiao_nrf54l15/nrf54l15/cpuapp -S nordic-flpr --no-sysbuild && west flash
west build -p -b xiao_nrf54l15/nrf54l15/cpuflpr --no-sysbuild && west flash
```

### Debug
```powershell
west build --build-dir <sample>/build <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild -- -DCONFIG_DEBUG=y
cd <sample>/build && west debug
```

**Key GDB commands:** `monitor reset halt` | `break main` | `continue` | `backtrace` | `info locals` | `x/20x $sp` | `quit`

### Recovery (Run Twice)
```powershell
python -m pyocd erase -u <SERIAL> -t nrf54l --mass -v
python -m pyocd erase -u <SERIAL> -t nrf54l --mass -v  # Second run performs erase
```
Find serial: `python -m pyocd list`

**Factory reset:** Use Seeed's tool from https://github.com/Jasionf/platform-seeedboards/tree/main/scripts/factory_reset in clean PowerShell (not nRF environment).

## Device Tree Aliases

```c
// Basic I/O
led0     -> GPIO2.0        gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
sw0      -> GPIO0.0        gpio_dt_spec btn = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

// Sensors & Audio
imu0     -> LSM6DSO (I2C 0x6A, i2c30)    device *imu = DEVICE_DT_GET(DT_ALIAS(imu0));
dmic20   -> PDM microphone                device *dmic = DEVICE_DT_GET(DT_ALIAS(dmic20));

// Xiao Connector
xiao_i2c     -> i2c22 (D4/D5)
xiao_spi     -> spi00 (D8/D9/D10)
xiao_serial  -> uart21 (D6/D7)
xiao_adc     -> adc (channels 0-7)

// RF Antenna Control (0=external U.FL, 1=internal ceramic)
rfsw_ctl -> GPIO2.5
```

**Pin access:** `DT_GPIO_PIN(DT_NODELABEL(xiao_d), N)` for D0-D10 (N=0 is GPIO1.4, N=1 is GPIO1.5, etc.)

## Project Structure

```
my_project/
├── CMakeLists.txt              # find_package(Zephyr) + target_sources
├── prj.conf                    # Kconfig options
├── src/main.c                  # #include <zephyr/kernel.h> + main()
└── boards/*.overlay            # Optional device tree mods
```

**Best practice:** Copy/modify existing samples rather than starting from scratch.

**Xiao-specific samples:** https://github.com/Toastee0/Seeed-Xiao-nRF54L15  
Comprehensive examples tested for Xiao nRF54L15: GPIO, IMU, DMIC, UART/I2C/SPI, Bluetooth, power management.

**Standard SDK samples:** `samples/basic/blinky`, `samples/bluetooth/`, `samples/sensor/` (may need board-specific config).

## Notes

- Multiple boards: Disconnect others (OpenOCD lacks `--dev-id`). JLink: use `--dev-id <SERIAL>`
- Multi-core flash: Each core uses separate flash regions, won't overwrite
- "Load failed" in GDB is normal when program running - use `continue`
- Match toolchain hash to SDK version (check install dates if multiple exist)