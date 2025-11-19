# nRF54L15 Development Guide

A comprehensive guide for building and flashing applications on the Seeed Studio Xiao nRF54L15.

## Prerequisites

- **nRF Connect SDK**: v2.7.0 or later (v3.2.0-preview2+ recommended for full nRF54L15 support)
- **Board**: Seeed Studio Xiao nRF54L15
- **OS**: Windows with PowerShell

## Quick Start

### 1. Environment Setup

Set these environment variables before any build command:

```powershell
$env:ZEPHYR_BASE = "C:\ncs\<VERSION>\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"
```

**Finding your SDK paths**: Replace `<VERSION>` and `<HASH>` with your installation-specific values:

**Find SDK versions**:
```powershell
Get-ChildItem C:\ncs\ | Where-Object {$_.Name -match '^v\d'}
```

**Find toolchain hash**:
```powershell
Get-ChildItem C:\ncs\toolchains\ | Where-Object {$_.Name -match '^[a-f0-9]+$'}
```

**Match toolchain to SDK version**: Each toolchain hash corresponds to a specific nRF SDK version. Use the toolchain that was installed with your chosen SDK version (check installation dates or verify the toolchain works with your `ZEPHYR_BASE` path).

### Setting Environment Variables Permanently

To avoid setting environment variables every session, configure them permanently in Windows:

**Method 1: System Properties (GUI)**
1. Press `Win + R`, type `sysdm.cpl`, press Enter
2. Click "Advanced" tab → "Environment Variables"  
3. Under "User variables" or "System variables", click "New"
4. Add each variable:
   - Variable name: `ZEPHYR_BASE`
   - Variable value: `C:\ncs\<VERSION>\zephyr`
   - Variable name: `ZEPHYR_TOOLCHAIN_VARIANT`  
   - Variable value: `zephyr`
   - Variable name: `ZEPHYR_SDK_INSTALL_DIR`
   - Variable value: `C:\ncs\toolchains\<HASH>\opt\zephyr-sdk`
5. Click OK to save

**Method 2: PowerShell (Permanent)**
```powershell
# Set permanent user environment variables (replace with your paths)
[Environment]::SetEnvironmentVariable("ZEPHYR_BASE", "C:\ncs\<VERSION>\zephyr", "User")
[Environment]::SetEnvironmentVariable("ZEPHYR_TOOLCHAIN_VARIANT", "zephyr", "User")  
[Environment]::SetEnvironmentVariable("ZEPHYR_SDK_INSTALL_DIR", "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk", "User")

# Restart PowerShell or VS Code to use the new variables
```

**Method 3: Command Prompt (Permanent)**
```cmd
setx ZEPHYR_BASE "C:\ncs\<VERSION>\zephyr"
setx ZEPHYR_TOOLCHAIN_VARIANT "zephyr"
setx ZEPHYR_SDK_INSTALL_DIR "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"
```

**Note**: After setting permanent variables, restart your terminal/IDE to use them. Once set, you can skip the `$env:` commands in build examples.

### 2. Board Targets

- **Application Core (ARM Cortex-M33)**: `xiao_nrf54l15/nrf54l15/cpuapp`
- **FLPR Core (RISC-V)**: `xiao_nrf54l15/nrf54l15/cpuflpr`

### 3. Basic Build & Flash

```powershell
# Set environment (replace <VERSION> and <HASH> with your values)
$env:ZEPHYR_BASE = "C:\ncs\<VERSION>\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"

# Build (replace <sample> with your project directory)
west build --build-dir <sample>/build <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild

# Flash
west flash --build-dir <sample>/build
```

## Build Methods

### Single Core Build

**Application Core Only**:
```powershell
west build --build-dir <sample>/build <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
```

**FLPR Core Only**:
```powershell
west build --build-dir <sample>/build_flpr <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuflpr --sysbuild
```

### Multi-Core Build

**Method 1: Sysbuild (Recommended)**

Builds both cores automatically. Requires sysbuild configuration files (`sysbuild.conf`, `Kconfig.sysbuild`, `sysbuild.cmake`).

```powershell
# Build both cores
west build --build-dir <sample>/build <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild

# Flash both images
west flash --build-dir <sample>/build
```

**Method 2: Separate Images**

For independent control when sysbuild isn't configured:

```powershell
# 1. Build app core with FLPR snippet
west build -p -b xiao_nrf54l15/nrf54l15/cpuapp -S nordic-flpr --no-sysbuild

# 2. Flash app core
west flash

# 3. Build FLPR core
west build -p -b xiao_nrf54l15/nrf54l15/cpuflpr --no-sysbuild

# 4. Flash FLPR core (images use separate flash regions)
west flash
```

## Flashing

### Basic Flash
```powershell
west flash --build-dir <sample>/build
```

### Multi-Core Flash
Each core's image only contains data for its flash region, so they don't overwrite each other when flashed normally.

### Multiple Boards
OpenOCD doesn't support `--dev-id`, so disconnect other boards when flashing.

For other runners (JLink):
```powershell
west flash --build-dir <sample>/build --dev-id <SERIAL_NUMBER>
```

## Recovery

### Unlock & Erase Bricked Board

⚠️ **Must run twice**: First unlocks APPROTECT, second performs mass erase.

```powershell
python -m pyocd erase -u <YOUR_SERIAL> -t nrf54l --mass -v
python -m pyocd erase -u <YOUR_SERIAL> -t nrf54l --mass -v
```

Find your board's serial number with: `python -m pyocd list`

### Seeed Studio Factory Reset Tool

**Important**: Must be run in a **clean PowerShell session**, not within nRF SDK environment.

Seeed Studio provides factory reset scripts available at:
**https://github.com/Jasionf/platform-seeedboards/tree/main/scripts/factory_reset**

The factory reset tool:
- Creates its own Python virtual environment
- Installs pyOCD and dependencies automatically  
- Runs mass erase and flashes factory firmware

**Usage**:
1. Download the factory reset scripts from the GitHub repository
2. Open a **new, clean PowerShell** (not nRF SDK terminal)
3. Navigate to the directory containing the factory reset files
4. Run the batch file: `.\factory_reset.bat`

The script automatically handles Python environment setup and dependency installation. Do not run this within the nRF Connect SDK environment as it may conflict with existing Python paths.

## Debugging

### Build with Debug Symbols
```powershell
# Set environment (replace <VERSION> and <HASH> with your values)
$env:ZEPHYR_BASE = "C:\ncs\<VERSION>\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"

# Build with debug symbols
west build --build-dir <sample>/build <sample> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild -- -DCONFIG_DEBUG=y
```

### Start Debug Session
```powershell
cd <sample>/build
west debug
```

### GDB Commands

**Basic Commands**:
```gdb
monitor reset halt    # Reset and halt
break main           # Breakpoint at main
continue            # Run program
backtrace           # Show call stack
backtrace full      # Call stack + variables
info registers      # Show all registers
list               # Show source code
quit               # Exit GDB
```

**Memory & Variables**:
```gdb
x/20x $sp              # Examine stack
x/20x 0x20000000      # Examine specific address
print variable_name    # Show variable value
info locals           # Show local variables
```

**Breakpoints & Stepping**:
```gdb
break function_name    # Break at function
break *0x12345        # Break at address
step                  # Step into
next                  # Step over
watch variable        # Break on change
```

**Tips**:
- "Load failed" is normal when program is running - use `continue`
- `monitor reset halt` restarts from debugger
- `info threads` shows Zephyr threads
- Ctrl+C interrupts running program

## Creating New Projects

### Required Project Structure

A proper nRF Connect SDK project requires these files:

```
my_project/
├── CMakeLists.txt          # Build configuration
├── prj.conf               # Kconfig settings
├── src/
│   └── main.c             # Application source code
└── boards/                # Board-specific configurations (optional)
    └── xiao_nrf54l15_nrf54l15_cpuapp.overlay
```

### Project Creation Steps

**1. Create project directories:**
```powershell
mkdir <project_name>
mkdir <project_name>/src
mkdir <project_name>/boards  # Optional for device tree overlays
```

**2. Create required files:**
- **CMakeLists.txt**: Standard Zephyr CMake configuration with `find_package(Zephyr)` and `target_sources(app PRIVATE src/main.c)`
- **prj.conf**: Kconfig options (see Common Kconfig Options below)
- **src/main.c**: Application entry point with `#include <zephyr/kernel.h>` and `main()` function
- **boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay**: Device tree modifications for board-specific features (optional)

**3. Use existing samples as reference:**
Most users should start by copying and modifying existing nRF Connect SDK samples rather than creating projects from scratch.

**Xiao nRF54L15 Specific Samples**: https://github.com/Toastee0/Seeed-Xiao-nRF54L15
This repository contains a comprehensive set of samples specifically customized and tested for the Xiao nRF54L15 board, including:
- Basic GPIO control (blink, button)
- Sensor integration (IMU, DMIC)  
- Communication protocols (UART, I2C, SPI)
- Bluetooth and wireless examples
- Power management and low-power examples

**Standard nRF Connect SDK samples** are also available but may require board-specific configuration.

### Xiao nRF54L15 Device Tree Aliases

The board definition provides these standard aliases for easy peripheral access:

#### Standard Zephyr Aliases
```c
// Basic I/O
#define LED0_NODE DT_ALIAS(led0)           // Built-in LED (GPIO2.0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define BUTTON0_NODE DT_ALIAS(sw0)         // User button (GPIO0.0) 
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);

// Sensors
#define IMU0_NODE DT_ALIAS(imu0)           // LSM6DSO IMU (I2C address 0x6A)
const struct device *imu = DEVICE_DT_GET(IMU0_NODE);

// Audio
#define DMIC_NODE DT_ALIAS(dmic20)         // PDM microphone
const struct device *dmic = DEVICE_DT_GET(DMIC_NODE);

// System
#define WDT_NODE DT_ALIAS(watchdog0)       // Watchdog timer (WDT31)

// RF Antenna Control
#define RFSW_NODE DT_NODELABEL(rfsw_ctl)   // RF switch control (GPIO2.5)
static const struct gpio_dt_spec rfsw_gpio = {
    .port = DEVICE_DT_GET(DT_GPIO_CTLR(RFSW_NODE, enable_gpios)),
    .pin = DT_GPIO_PIN(RFSW_NODE, enable_gpios),
    .dt_flags = DT_GPIO_FLAGS(RFSW_NODE, enable_gpios),
};
```

#### Xiao Connector Aliases
```c
// Xiao connector references (for shields/expansions)
xiao_i2c     -> &i2c22     // I2C on pins D4/D5  
xiao_spi     -> &spi00     // SPI on pins D8/D9/D10
xiao_serial  -> &uart21    // UART on pins D6/D7
xiao_adc     -> &adc       // ADC channels 0-7

// Direct pin access via Xiao connector
#define XIAO_D0  DT_GPIO_PIN(DT_NODELABEL(xiao_d), 0)  // GPIO1.4
#define XIAO_D1  DT_GPIO_PIN(DT_NODELABEL(xiao_d), 1)  // GPIO1.5
// ... pins D0-D10 available
```

#### Available Peripherals
- **UARTs**: `uart20` (console), `uart21` (Xiao pins D6/D7)  
- **I2C**: `i2c22` (Xiao pins D4/D5), `i2c30` (internal IMU)
- **SPI**: `spi00` (Xiao pins D8/D9/D10), `spi22`
- **ADC**: 8 channels (0-7) accessible via Xiao connector
- **PDM**: `pdm20` (digital microphone)
- **GPIO**: All pins accessible via port/pin or Xiao connector aliases

#### Usage Examples from Working Samples
```c
// LED control (from blink sample)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
gpio_pin_toggle_dt(&led);

// Button input (from button sample)  
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
gpio_pin_configure_dt(&button, GPIO_INPUT);
int state = gpio_pin_get_dt(&button);

// IMU access (from imu sample)
const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c30));
// LSM6DSO at address 0x6A on internal I2C bus

// DMIC audio (from dmic sample)
const struct device *dmic = DEVICE_DT_GET(DT_ALIAS(dmic20));

// RF antenna switching (from rfsw sample)
static const struct gpio_dt_spec rfsw_gpio = {
    .port = DEVICE_DT_GET(DT_GPIO_CTLR(DT_NODELABEL(rfsw_ctl), enable_gpios)),
    .pin = DT_GPIO_PIN(DT_NODELABEL(rfsw_ctl), enable_gpios),
    .dt_flags = DT_GPIO_FLAGS(DT_NODELABEL(rfsw_ctl), enable_gpios),
};
gpio_pin_configure_dt(&rfsw_gpio, GPIO_OUTPUT_ACTIVE);
// gpio_pin_set_dt(&rfsw_gpio, 0) = External antenna (U.FL connector)
// gpio_pin_set_dt(&rfsw_gpio, 1) = Internal ceramic antenna
```



## Examples

### Complete Build & Flash Workflow
```powershell
# Set environment (replace <VERSION> and <HASH> with your values)
$env:ZEPHYR_BASE = "C:\ncs\<VERSION>\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"

# Build (using blinky sample as example)
west build --build-dir samples/basic/blinky/build samples/basic/blinky --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild

# Flash
west flash --build-dir samples/basic/blinky/build


```

### Debug Session Example
```powershell
# Build with debug symbols (using blinky sample)
west build --build-dir samples/basic/blinky/build samples/basic/blinky --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild -- -DCONFIG_DEBUG=y

# Start debugging
cd samples/basic/blinky/build
west debug

# In GDB:
# (gdb) monitor reset halt
# (gdb) break main
# (gdb) continue
```

### Building Custom Projects

```powershell
# Set environment (replace <VERSION> and <HASH> with your values)
$env:ZEPHYR_BASE = "C:\ncs\<VERSION>\zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\<HASH>\opt\zephyr-sdk"

# Build your project
west build --build-dir <project>/build <project> --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild

# Flash
west flash --build-dir <project>/build
```

**Recommended starting points:**
- Copy and modify existing nRF Connect SDK samples from `samples/` directory
- Reference `samples/basic/blinky` for minimal GPIO project structure  
- Use `samples/bluetooth/` for BLE applications
- Check `samples/sensor/` for peripheral integration examples