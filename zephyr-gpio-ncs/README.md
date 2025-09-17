# GPIO Button to Relay Control Sample

This sample demonstrates GPIO input and output operations by reading a button state and controlling a relay accordingly on the Seeed XIAO nRF54L15.

## Features

- GPIO input for button reading (sw1)
- GPIO output for relay control (relay0)
- Real-time button state monitoring
- Console logging for status updates
- Error handling for GPIO operations

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- External button connected to sw1 GPIO pin
- Relay module connected to relay0 GPIO pin

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

When you press the button (sw1), the relay will activate. When you release the button, the relay will deactivate. Console output will show:

```
Starting Zephyr button and relay example...
Press the button to toggle the relay...
```

## Configuration

- GPIO is enabled for both button input and relay output
- Logging is enabled at info level for status monitoring
- Uses device tree aliases `sw1` for button and `relay0` for relay
- Requires appropriate device tree overlay to define the GPIO pins

## Hardware Setup

Make sure to define the `sw1` and `relay0` aliases in your device tree overlay file with the appropriate GPIO pins for your hardware setup.