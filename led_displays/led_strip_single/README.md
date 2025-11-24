# Single RGB LED Smooth Glow

This sample provides a smooth, gentle glowing animation designed for a single RGB LED consisting of three separate LEDs (one red, one green, one blue).

## Features

### Animation Effect

A single continuous smooth glow effect that cycles through three colors:

- **Red → Blue** - 500ms smooth linear fade
- **Blue → Green** - 500ms smooth linear fade  
- **Green → Red** - 500ms smooth linear fade

**Characteristics:**
- 25% brightness (64/255) for gentle ambient lighting
- No flashing or strobing - pure smooth transitions
- 1.5 second complete cycle time
- Continuous loop

### Customization

Edit `prj.conf` or use Kconfig to adjust:

- `CONFIG_SAMPLE_LED_UPDATE_DELAY` - Animation update rate (default: 20ms for 50 FPS)
- `CONFIG_SAMPLE_LED_BRIGHTNESS` - Maximum brightness level (default: 255, used at 25% = 64)

## Hardware Requirements

- An RGB LED setup with three individual LEDs (R, G, B) connected to a WS2812-compatible driver
- The device tree should define a `led-strip` alias pointing to your LED strip device
- Chain length should be set to 1 in the device tree overlay

## Building and Running

```bash
west build -b xiao_nrf54l15_nrf54l15_cpuapp
west flash
```

## Device Tree Configuration

The sample expects a device tree configuration like:

```dts
&xiao_spi {
    led_strip: ws2812@0 {
        compatible = "worldsemi,ws2812-spi";
        reg = <0>;
        spi-max-frequency = <...>;
        chain-length = <1>;  /* Single RGB LED */
        color-mapping = <LED_COLOR_ID_GREEN
                         LED_COLOR_ID_RED
                         LED_COLOR_ID_BLUE>;
        spi-one-frame = <...>;
        spi-zero-frame = <...>;
    };
};

/ {
    aliases {
        led-strip = &led_strip;
    };
};
```

## Technical Details

- Uses linear interpolation (lerp) for smooth color transitions
- Direct RGB color control at 25% brightness
- Optimized for single pixel displays
- Very low CPU usage (~0.1%)
- 50 FPS update rate (20ms) for perfectly smooth fades

## Customizing the Animation

### Change Colors
Edit the color definitions in `src/main.c`:
```c
#define COLOR_RED    {.r = GLOW_BRIGHTNESS, .g = 0, .b = 0}
#define COLOR_BLUE   {.r = 0, .g = 0, .b = GLOW_BRIGHTNESS}
#define COLOR_GREEN  {.r = 0, .g = GLOW_BRIGHTNESS, .b = 0}
```

### Change Brightness
Adjust `GLOW_BRIGHTNESS` calculation:
```c
#define GLOW_BRIGHTNESS (MAX_BRIGHTNESS / 4)  /* 25% = /4, 50% = /2, etc. */
```

### Change Fade Speed
Modify `steps_per_transition` in the animation function:
```c
const uint32_t steps_per_transition = 25;  /* 500ms at 20ms/step */
// For 1 second: 50 steps
// For 250ms: 12-13 steps
```

## Notes

- Gentle 25% brightness for ambient/mood lighting
- No harsh transitions or flashing
- Perfectly smooth color fades
- Continuous operation
- Uses LED strip driver for easy hardware compatibility
