# PWM LED Fade Sample

This sample demonstrates PWM (Pulse Width Modulation) control by creating a smooth LED fade in/out effect on the Seeed XIAO nRF54L15.

## Features

- PWM signal generation at 1kHz frequency
- Smooth LED brightness control using duty cycle modulation
- Automatic fade in/out effect
- Console logging for status monitoring
- Error handling for PWM operations

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- LED connected to PWM output (uses pwm_led alias from device tree)

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The LED will continuously fade in and out with a smooth transition. Console output will show:

```
Starting Zephyr LED fade example...
PWM Period set to 1000000 ns (1kHz frequency)
```

## Configuration

- PWM driver is enabled for LED control
- Console output and logging are enabled for debugging
- PWM operates at 1kHz (1,000,000 nanosecond period)
- Duty cycle varies from 0% to 100% for fade effect
- Uses device tree alias `pwm_led` for PWM configuration

## Technical Details

- PWM Period: 1ms (1kHz frequency)
- Fade resolution: 85 steps (0-255 in increments of 3)
- Fade speed: 30ms per step
- Complete fade cycle: ~5.1 seconds