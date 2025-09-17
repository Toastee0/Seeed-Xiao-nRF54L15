# Basic LED Blink Sample

This sample demonstrates basic GPIO output functionality by blinking the onboard LED on the Seeed XIAO nRF54L15.

## Features

- GPIO output control
- Basic LED blinking at 1 Hz
- Low power mode request
- Simple error handling

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- Onboard LED (uses led0 alias from device tree)

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The onboard LED will blink on and off every second. No console output is generated as serial communication is disabled for power optimization.

## Configuration

- GPIO is enabled for LED control
- Serial/console output is disabled for minimal power consumption
- Uses device tree alias `led0` for LED configuration