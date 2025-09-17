# Button to LED Control Sample

This sample demonstrates GPIO input and output by reading a button state and controlling an LED accordingly on the Seeed XIAO nRF54L15.

## Features

- GPIO input for button reading
- GPIO output for LED control
- Real-time button state monitoring
- Console logging for status updates
- Error handling for GPIO operations

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- Onboard button (uses sw0 alias from device tree)
- Onboard LED (uses led0 alias from device tree)

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

When you press the button, the LED will turn on. When you release the button, the LED will turn off. Console output will show:

```
Starting Zephyr button and led example...
Press the button to toggle the led...
```

## Configuration

- GPIO is enabled for both button input and LED output
- Logging is enabled at info level for status monitoring
- Uses device tree aliases `sw0` for button and `led0` for LED