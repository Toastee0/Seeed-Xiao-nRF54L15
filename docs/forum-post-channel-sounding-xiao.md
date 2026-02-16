# Bluetooth Channel Sounding (Distance Estimation) on Seeed Studio Xiao nRF54L15

## Overview

I've ported the Nordic `channel_sounding_ras_initiator` and `channel_sounding_ras_reflector` samples to run on the **Seeed Studio Xiao nRF54L15** board. This gives you BLE Channel Sounding distance estimation between two Xiao boards (or one Xiao and an nRF54L15 DK) using the RAS (Ranging Service) profile.

The samples are tested and building cleanly on **nRF Connect SDK v3.2.1**.

**Repository:** https://github.com/Toastee0/Seeed-Xiao-nRF54L15

The samples are located at:
- `bluetooth/channel_sounding_ras_initiator/` - Scans for and connects to a reflector, runs CS procedures, estimates distance
- `bluetooth/channel_sounding_ras_reflector/` - Advertises the Ranging Service, responds to CS procedures

## What Was Changed from Stock SDK Samples

The stock Nordic samples target the nRF54L15 DK which has the DK library (4 LEDs, 4 buttons) and specific board configs. The Xiao nRF54L15 is a much smaller board with a single user LED and an RF antenna switch that needs manual configuration. Here's what was modified:

### 1. Replaced DK Library with Direct GPIO

The DK uses `#include <dk_buttons_and_leds.h>` with `DK_LED1`. The Xiao has one LED on `led0`, so I replaced it with direct GPIO control:

```c
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static inline int dk_leds_init(void) {
    if (!gpio_is_ready_dt(&led0)) return -ENODEV;
    return gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
}

static inline void dk_set_led_on(uint8_t led) {
    gpio_pin_set_dt(&led0, 1);
}

static inline void dk_set_led_off(uint8_t led) {
    gpio_pin_set_dt(&led0, 0);
}
```

The LED turns on when connected to the peer device.

### 2. RF Antenna Switch Configuration

The Xiao nRF54L15 has an RF switch that must be configured before Bluetooth initialization. Without this, the radio won't work properly. This is added in `main()` before `bt_enable()`:

```c
const struct device *gpio2 = DEVICE_DT_GET(DT_NODELABEL(gpio2));
if (device_is_ready(gpio2)) {
    gpio_pin_configure(gpio2, 3, GPIO_OUTPUT_ACTIVE);   /* rfsw-pwr = HIGH (enable switch) */
    gpio_pin_configure(gpio2, 5, GPIO_OUTPUT_INACTIVE); /* rfsw-ctl = LOW (onboard antenna) */
}
```

- `GPIO2.3` (rfsw-pwr): HIGH enables the antenna switch
- `GPIO2.5` (rfsw-ctl): LOW = onboard ceramic antenna, HIGH = external U.FL connector

### 3. prj.conf Adapted for Xiao

The stock samples use `CONFIG_NCS_SAMPLES_DEFAULTS=y` and `CONFIG_DK_LIBRARY=y` which pull in DK-specific defaults. For the Xiao I replaced these with explicit configs:

```
# Xiao doesn't have DK library - use direct GPIO + explicit logging
CONFIG_GPIO=y
CONFIG_REBOOT=y
CONFIG_BT_LL_SW_SPLIT=n    # Force SoftDevice Controller

# Console and logging (not included in NCS_SAMPLES_DEFAULTS on Xiao)
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_LOG=y
CONFIG_LOG_MODE_IMMEDIATE=y
CONFIG_LOG_DEFAULT_LEVEL=3
```

Everything else (BT configs, CS configs, MTU settings, etc.) is identical to the stock samples.

### 4. Initiator: Calibrated Distance Output (Bonus)

The initiator sample adds calibration and weighted averaging on top of the raw distance estimates. This is optional but improves real-world accuracy:

- **Phase Slope** (most stable): Weight 50%, calibrated with measured slope/offset
- **RTT**: Weight 25%
- **IFFT**: Weight 25%

Output shows all three methods plus a weighted best estimate:
```
[AP0] IFFT:1.23m Phase:0.98m RTT:1.45m -> Best:1.12m
```

The calibration constants were derived from measurements at 0.5m, 1m, 2m, 3m, and 4m. You'll likely want to tune these for your own environment.

## Hardware Required

- **2x Seeed Studio Xiao nRF54L15** (one initiator, one reflector) - OR one Xiao + one nRF54L15 DK
- USB-C cables for power/programming/serial console

## Build Instructions

### Prerequisites
- nRF Connect SDK v3.2.1 installed
- `west` and toolchain configured

### Environment Setup (PowerShell)
```powershell
$env:ZEPHYR_BASE = "C:\ncs\v3.2.1\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<YOUR_HASH>\opt\zephyr-sdk"
```

Find your toolchain hash: `Get-ChildItem C:\ncs\toolchains\ | Where-Object {$_.Name -match '^[a-f0-9]+$'}`

### Clone the Repository
```bash
git clone https://github.com/Toastee0/Seeed-Xiao-nRF54L15.git
cd Seeed-Xiao-nRF54L15
```

### Build the Reflector (flash to Board #1)
```powershell
west build --build-dir bluetooth/channel_sounding_ras_reflector/build bluetooth/channel_sounding_ras_reflector --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
west flash --build-dir bluetooth/channel_sounding_ras_reflector/build
```

### Build the Initiator (flash to Board #2)
```powershell
west build --build-dir bluetooth/channel_sounding_ras_initiator/build bluetooth/channel_sounding_ras_initiator --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
west flash --build-dir bluetooth/channel_sounding_ras_initiator/build
```

### Monitor Serial Output
Connect a serial terminal (PuTTY, minicom, or `nRF Connect Serial Terminal`) at **115200 baud** to see the distance estimates on the initiator board.

## Build Sizes (SDK v3.2.1)

| Sample | FLASH | RAM |
|--------|-------|-----|
| Reflector | 250 KB (17% of 1428 KB) | 58 KB (30% of 188 KB) |
| Initiator | 298 KB (20% of 1428 KB) | 73 KB (28% of 256 KB) |

## Expected Behavior

1. Power on the **reflector** first - it starts advertising the Ranging Service
2. Power on the **initiator** - it scans for the Ranging Service UUID, connects, and starts CS procedures
3. The LED on both boards turns on when connected
4. The initiator serial console prints distance estimates continuously
5. If either board disconnects, it reboots automatically to restart

## Tips & Troubleshooting

- **No connection?** Make sure the reflector is powered on and advertising before starting the initiator. Check that the RF switch is configured (you should see "RF switch configured for onboard antenna" in the log).
- **Distance values seem wrong?** The calibration constants in the initiator are tuned for my specific environment. You may need to adjust `PHASE_SLOPE_SLOPE`, `RTT_SLOPE`, etc. based on your own measurements at known distances.
- **Want to use the external antenna?** Change `GPIO_OUTPUT_INACTIVE` to `GPIO_OUTPUT_ACTIVE` on the `rfsw-ctl` pin (GPIO2.5) in both samples.
- **Recovery if board is bricked:** Run mass erase twice:
  ```
  python -m pyocd erase -t nrf54l --mass -v
  python -m pyocd erase -t nrf54l --mass -v
  ```

## Other Samples in the Repo

The repository also includes many other Xiao nRF54L15 samples: GPIO blink, IMU (LSM6DSO), PDM microphone, UART/I2C/SPI, BLE peripheral/central, power management, and more. See the repo README for the full list.
