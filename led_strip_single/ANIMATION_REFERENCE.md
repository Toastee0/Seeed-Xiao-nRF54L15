# Animation Reference Card

## Quick Animation Overview

| Animation      | Speed      | Effect Description                              | Key Parameters          |
|----------------|------------|-------------------------------------------------|-------------------------|
| Rainbow Fade   | Medium     | Smooth color wheel rotation                     | Hue cycles 0-255       |
| Breathe        | Slow       | Pulsing brightness with color change            | Sine wave modulation   |
| Strobe         | Fast       | Quick on/off flashes with color changes         | 2 frames on, 8 off     |
| Color Wipe     | Very Slow  | Solid colors holding for 2 seconds each         | 7 preset colors        |
| Pulse          | Medium     | Quick brightness ramps with fast color cycling  | Triangle wave 20 steps |

## Color Progression by Animation

### Rainbow Fade
Continuous smooth spectrum: Red → Orange → Yellow → Green → Cyan → Blue → Magenta → Red...

### Breathe
Slow color cycling while brightness oscillates smoothly (sine wave)

### Strobe
Rapid color jumps: Red → Orange → Yellow → Green → Cyan → Blue → Magenta (held briefly)

### Color Wipe
Solid colors held ~2 seconds each:
1. Red (Hue 0)
2. Orange (Hue 21)
3. Yellow (Hue 43)
4. Green (Hue 85)
5. Cyan (Hue 128)
6. Blue (Hue 170)
7. Magenta (Hue 213)

### Pulse
Fast rainbow color cycling with triangular brightness pulses

## Timing Information

**Default Settings:**
- Update Rate: 20ms (50 FPS)
- Animation Duration: 10 seconds each
- Total Cycle Time: 50 seconds (5 animations × 10 seconds)

**Steps per Animation:**
- Rainbow Fade: 500 steps (10s ÷ 20ms)
- Breathe: 500 steps
- Strobe: 500 steps
- Color Wipe: 500 steps (≈71 steps per color)
- Pulse: 500 steps (25 complete pulses)

## Customization Tips

### Make Animations Slower
```c
// In prj.conf or Kconfig
CONFIG_SAMPLE_LED_UPDATE_DELAY=50  // 20 FPS instead of 50 FPS
```

### Make Animations Brighter
```c
CONFIG_SAMPLE_LED_BRIGHTNESS=255  // Maximum brightness
```

### Change Animation Duration
```c
// In src/main.c, modify the switch condition:
if (anim_duration >= 20000 / CONFIG_SAMPLE_LED_UPDATE_DELAY) {
    // Now 20 seconds instead of 10
```

### Disable Specific Animations
Comment out unwanted animations in the switch statement and adjust `ANIM_COUNT`

## Animation Algorithm Details

### Rainbow Fade
```c
hue = (step * 2) % 256;  // 2x speed through color wheel
hsv_to_rgb(hue, 255, MAX_BRIGHTNESS, &pixel);
```

### Breathe
```c
hue = (step / 4) % 256;  // Slower color change
brightness = (sin(step * 0.05) + 1.0) / 2.0 * MAX_BRIGHTNESS;
```

### Strobe
```c
hue = ((step / 10) * 30) % 256;  // Jump 30 degrees every 10 frames
brightness = (step % 10 < 2) ? MAX : 0;  // On for 2/10 frames
```

### Color Wipe
```c
color_idx = (step / 100) % 7;  // Hold each color for 100 frames
hsv_to_rgb(preset_hues[color_idx], 255, MAX);
```

### Pulse
```c
hue = (step / 3) % 256;  // Fast color cycling
pulse_pos = step % 20;   // 20-frame triangle wave
if (pulse_pos < 10)
    brightness = pulse_pos * MAX / 10;  // Ramp up
else
    brightness = (20 - pulse_pos) * MAX / 10;  // Ramp down
```

## HSV Color Space

The animations use HSV (Hue, Saturation, Value) for natural color transitions:

- **Hue**: 0-255 (color wheel position)
  - 0 = Red
  - 43 = Yellow
  - 85 = Green
  - 128 = Cyan
  - 170 = Blue
  - 213 = Magenta

- **Saturation**: 0-255 (color intensity)
  - 0 = White/Gray
  - 255 = Full color (used in all animations)

- **Value**: 0-255 (brightness)
  - 0 = Off
  - 128 = Half brightness (default max)
  - 255 = Full brightness

## Performance Notes

- HSV to RGB conversion: ~150 CPU cycles per update
- Sine calculation (breathe only): ~200 CPU cycles
- Total CPU usage: <1% on nRF54L15 @ 50 FPS
- Memory usage: Minimal (single pixel buffer)
