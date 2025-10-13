# PWM RGB LED Sample# PWM LED Fade Sample



This sample demonstrates RGB LED control using PWM (Pulse Width Modulation) on three consecutive channels of the Seeed XIAO nRF54L15.This sample demonstrates PWM (Pulse Width Modulation) control by creating a smooth LED fade in/out effect on the Seeed XIAO nRF54L15.



## Features## Features



- **3-channel PWM control** for RGB LED- PWM signal generation at 1kHz frequency

- **Rainbow effect** using HSV to RGB color conversion- Smooth LED brightness control using duty cycle modulation

- **Primary and secondary color display**- Automatic fade in/out effect

- **Smooth fade effects** (white fade, black fade)- Console logging for status monitoring

- **1 kHz PWM frequency** on all channels- Error handling for PWM operations

- Console logging for debugging

## Hardware Requirements

## Hardware Requirements

- Seeed XIAO nRF54L15 development board

- Seeed XIAO nRF54L15 development board- LED connected to PWM output (uses pwm_led alias from device tree)

- RGB LED (common cathode or 3 separate LEDs with common ground)

- 3x current-limiting resistors (typically 220Ω-330Ω depending on LED)## Building and Running



## WiringThis sample is built using the Nordic Connect SDK (NCS). To build:



Connect the RGB LED to the following pins:```bash

west build -b xiao_nrf54l15/nrf54l15/cpuapp

| LED Color | XIAO Pin | GPIO Pin | PWM Channel |west flash

|-----------|----------|----------|-------------|```

| Red       | D1       | P1.05    | PWM20 CH1   |

| Green     | D2       | P1.06    | PWM20 CH2   |## Expected Output

| Blue      | D3       | P1.07    | PWM20 CH3   |

The LED will continuously fade in and out with a smooth transition. Console output will show:

**Note**: Connect appropriate current-limiting resistors in series with each LED channel.

```

## Building and RunningStarting Zephyr LED fade example...

PWM Period set to 1000000 ns (1kHz frequency)

```bash```

west build -b xiao_nrf54l15/nrf54l15/cpuapp

west flash## Configuration

```

- PWM driver is enabled for LED control

## Expected Behavior- Console output and logging are enabled for debugging

- PWM operates at 1kHz (1,000,000 nanosecond period)

The RGB LED will cycle through several effects in a loop:- Duty cycle varies from 0% to 100% for fade effect

- Uses device tree alias `pwm_led` for PWM configuration

1. **Rainbow Effect** - Smooth color transition through entire color spectrum (HSV hue rotation)

2. **Fade to White** - Gradually increase all channels to full brightness## Technical Details

3. **Fade to Black** - Gradually decrease all channels to off

4. **Primary Colors** - Display Red → Green → Blue in sequence- PWM Period: 1ms (1kHz frequency)

5. **Secondary Colors** - Display Yellow → Cyan → Magenta in sequence- Fade resolution: 85 steps (0-255 in increments of 3)

- Fade speed: 30ms per step

## Console Output- Complete fade cycle: ~5.1 seconds

```
[00:00:00.123,456] <inf> pwm_rgb_example: Starting RGB LED PWM example...
[00:00:00.123,678] <inf> pwm_rgb_example: All PWM channels ready. Period: 1000000 ns (1kHz)
[00:00:00.123,890] <inf> pwm_rgb_example: RGB LED on D1 (Red), D2 (Green), D3 (Blue)
[00:00:07.234,567] <inf> pwm_rgb_example: Fading to white...
[00:00:08.345,678] <inf> pwm_rgb_example: Fading to black...
[00:00:09.456,789] <inf> pwm_rgb_example: Primary colors...
[00:00:12.567,890] <inf> pwm_rgb_example: Secondary colors...
```

## Configuration

- **PWM Period**: 1ms (1,000,000 nanoseconds) = 1 kHz frequency
- **PWM Channels**: 3 independent channels on PWM20 instance
- **Color Resolution**: 8-bit per channel (0-255)
- **HSV Color Space**: Used for smooth rainbow transitions
- **Floating Point**: Enabled for HSV to RGB conversion

## Technical Details

### PWM Configuration
- Uses Nordic PWM20 peripheral
- 3 consecutive output channels (CHAN[1], CHAN[2], CHAN[3])
- Independent duty cycle control per channel
- Low-power sleep state configuration included

### Color Control
- `set_rgb_color(r, g, b)` - Set color with 8-bit RGB values (0-255)
- `hsv_to_rgb(h, s, v)` - Convert HSV color space to RGB
  - H (Hue): 0-360 degrees
  - S (Saturation): 0-100%
  - V (Value/Brightness): 0-100%

## Code Structure

```
pwm_rgb/
├── CMakeLists.txt              # Build configuration
├── prj.conf                    # Project configuration (PWM + FPU enabled)
├── README.md                   # This file
├── sample.yaml                 # Sample metadata
├── boards/
│   └── xiao_nrf54l15_nrf54l15_cpuapp.overlay  # Board-specific PWM pin configuration
└── src/
    └── main.c                  # RGB LED control with effects
```

## Extending the Sample

You can easily add your own color effects by calling `set_rgb_color()`:

```c
// Custom purple color
set_rgb_color(128, 0, 128);

// Breathing effect on single color
for (int i = 0; i <= 255; i += 5) {
    set_rgb_color(i, 0, 0);  // Red breathing
    k_msleep(20);
}
```

## PWM-Capable Pins on XIAO nRF54L15

All available PWM channels:
- D0 (P1.04): PWM20 CHAN[0]
- D1 (P1.05): PWM20 CHAN[1] ← Used for Red
- D2 (P1.06): PWM20 CHAN[2] ← Used for Green
- D3 (P1.07): PWM20 CHAN[3] ← Used for Blue
- D4 (P1.10): PWM20 CHAN[4]
- D5 (P1.11): PWM20 CHAN[5]

## Troubleshooting

- **LED not lighting**: Check resistor values and LED polarity
- **Wrong colors**: Verify RGB LED pinout and wiring
- **Erratic behavior**: Ensure common ground connection
- **Build errors**: Verify FPU and NEWLIB_LIBC configs are enabled
