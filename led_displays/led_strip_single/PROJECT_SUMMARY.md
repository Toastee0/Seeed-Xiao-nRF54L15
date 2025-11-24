# LED Strip Single - Project Summary

## What Was Created

A new sample project `led_strip_single` has been created based on the `led_strip` sample, specifically designed for a single RGB LED with smooth, bright animations.

## Project Location

```
m:\nRF54L15\led_strip_single\
```

## Key Changes from Original

### 1. Main Application (`src/main.c`)

**New Features:**
- 5 different animation types that automatically cycle every 10 seconds
- HSV to RGB color conversion for smooth color transitions
- Uses only 1 pixel instead of multiple pixels
- Animations designed for slow, smooth, visually appealing effects

**Animation Types:**
1. **Rainbow Fade** - Continuous smooth color spectrum cycling
2. **Breathe** - Sine wave brightness with color cycling
3. **Strobe** - Quick flash effects
4. **Color Wipe** - Solid colors that hold
5. **Pulse** - Rhythmic pulsing with rainbow

### 2. Configuration (`Kconfig`)

**Updated Defaults:**
- Update delay: 50ms → **20ms** (50 FPS for smoother animations)
- Brightness: 16 → **128** (much brighter for better visibility)
- Updated descriptions to reflect single RGB LED usage

### 3. CMakeLists.txt

**Added:**
- Project name changed to `led_strip_single`
- Math library linked for sine wave calculations

### 4. Documentation

**Created:**
- `README.md` - Comprehensive project documentation
- Updated `README.rst` with single RGB LED focus
- Updated `sample.yaml` with new description

### 5. Device Tree Overlay

**Updated:**
- Added comment clarifying this is for a single RGB LED (3 LEDs: R, G, B)
- Chain length is already 1 (perfect for single LED)

## How to Build

```bash
cd m:\nRF54L15\led_strip_single
west build -b xiao_nrf54l15_nrf54l15_cpuapp
west flash
```

## Customization Options

### Via Kconfig (`prj.conf` or menuconfig):

```
CONFIG_SAMPLE_LED_UPDATE_DELAY=20    # Animation speed (ms)
CONFIG_SAMPLE_LED_BRIGHTNESS=128     # Max brightness (1-255)
```

### Adding Custom Animations:

1. Add new animation function in `src/main.c`
2. Add enum entry to `animation_type`
3. Add case to switch statement
4. Update animation names array

## Technical Highlights

- **Smooth Color Transitions**: Uses HSV color space for natural color progression
- **Mathematical Effects**: Implements sine waves for breathing effects
- **Low CPU Usage**: Efficient animation calculations
- **High Brightness**: Default 128/255 for good visibility
- **Auto-Cycling**: Changes animations every 10 seconds
- **Scalable**: Easy to modify for multiple LEDs if needed

## Hardware Compatibility

- Designed for XIAO nRF54L15 board
- Uses WS2812 LED protocol via SPI
- Single RGB LED (three separate R, G, B LEDs)
- Connected to P2.08 (D6/TX) pin

## File Structure

```
led_strip_single/
├── CMakeLists.txt              # Build configuration
├── Kconfig                     # Configuration options
├── prj.conf                    # Project configuration
├── sample.yaml                 # Sample metadata
├── nrf54L15-bindings.h        # Hardware bindings
├── README.md                   # User documentation
├── README.rst                  # Zephyr documentation
├── boards/
│   ├── xiao_nrf54l15_nrf54l15_cpuapp.conf
│   └── xiao_nrf54l15_nrf54l15_cpuapp.overlay  # Device tree
└── src/
    └── main.c                  # Main application with animations
```

## Next Steps

1. Review the animations in `src/main.c`
2. Adjust brightness/speed in `Kconfig` if needed
3. Build and flash to your board
4. Watch the animations cycle!
5. Add custom animations as desired

## Notes

- All build artifacts (build/, build_xiao/) have been removed from the copy
- The project is ready to build from scratch
- Configuration is optimized for slow, smooth, bright animations
- Easy to extend with additional animation patterns
