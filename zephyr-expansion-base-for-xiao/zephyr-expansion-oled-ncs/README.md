# Expansion Board OLED Display Sample

This sample demonstrates how to use the SSD1306 OLED display with the Seeed XIAO nRF54L15 and Grove Expansion Base. The sample uses Zephyr's Character Framebuffer (CFB) API to display text on the 128x64 monochrome OLED screen.

## Features
- SSD1306 OLED display driver (128x64 resolution)
- Character Framebuffer (CFB) for text rendering
- I2C communication interface
- Automatic font selection and sizing
- Text positioning by row and column
- Real-time display updates

## Hardware Requirements
- Seeed XIAO nRF54L15 development board
- Grove Expansion Base for XIAO
- Grove OLED Display 0.96" (SSD1306, 128x64)
- Grove cable for connection

## Display Connection
The OLED display connects to the expansion base via I2C:

| OLED Pin | Expansion Base | Function |
|----------|----------------|----------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SDA | SDA (I2C Data) | I2C Data Line |
| SCL | SCL (I2C Clock) | I2C Clock Line |

**I2C Address**: 0x3C (default for most SSD1306 displays)

## Building and Running

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

### Display Content:
The OLED will show:
- "nRF54L15" at row 1, column 2
- "Hello World" at row 2, column 1
- Text updates every second

### Console Output:
```
[00:00:00.123,456] <inf> main_app: Initialized SSD1306@3c
[00:00:00.234,567] <inf> main_app: Selected font idx: 0, width: 8, height: 8
[00:00:00.345,678] <inf> main_app: Display resolution: 128x64
```

## Technical Details
- **Display Controller**: SSD1306
- **Resolution**: 128x64 pixels (monochrome)
- **Interface**: I2C at standard speed (100kHz)
- **Font**: Automatically selected 8x8 pixel font
- **Framebuffer**: Character-based rendering system
- **Update Rate**: 1 Hz (1 second intervals)

## Display Configuration
The device tree overlay configures:
- I2C interface on XIAO I2C pins
- SSD1306 controller at address 0x3C
- Display dimensions (128x64)
- Segment and page mapping
- Multiplex ratio and precharge settings

## Character Framebuffer Features
- **Font Selection**: Automatic font detection and selection
- **Text Positioning**: Row/column based positioning system
- **Display Management**: Clear, print, and finalize operations
- **Kerning Control**: Adjustable character spacing

## Customization
To modify the display content:
1. Edit text strings in `main.c`
2. Change positioning using `print_text_by_row_col()`
3. Adjust update timing with `k_sleep()` duration
4. Add graphics using CFB drawing functions

## Troubleshooting
- **Display not detected**: Check I2C connections and power supply
- **Garbled text**: Verify I2C address (0x3C) and display compatibility
- **Font issues**: Check font availability and selection logic
- **No display updates**: Ensure `cfb_framebuffer_finalize()` is called
- **I2C errors**: Verify SDA/SCL connections and pull-up resistors

## Compatible Displays
- Grove OLED Display 0.96" (SSD1306)
- Generic SSD1306 128x64 I2C OLED displays
- Other Solomon Systech SSD1306-based displays