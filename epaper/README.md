# E-Paper Display Sample

This sample demonstrates how to use an e-paper display with the Seeed XIAO nRF54L15 using LVGL graphics library. The sample creates a simple graphical interface with text labels and geometric shapes optimized for e-paper displays.

## Features
- E-paper display driver integration
- LVGL graphics library usage
- Monochrome (1-bit) display support
- Static content display with various fonts
- Custom GUI elements (panels, labels, shapes)

## Hardware Requirements
- Seeed XIAO nRF54L15 development board
- Compatible e-paper display (7.5" 800x480, UC8179 controller)
- SPI connection between board and display

## Display Connection
The e-paper display connects via SPI with additional control pins:

| Display Pin | XIAO Pin | Function |
|-------------|----------|----------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |  
| DIN | MOSI (D10) | SPI Data In |
| CLK | SCK (D8) | SPI Clock |
| CS | D1 | Chip Select |
| DC | D3 | Data/Command |
| RST | D0 | Reset |
| BUSY | D2 | Busy indicator |

## Building and Running

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The display will show:
- "HELLO EPAPER" text in a centered bordered panel
- "Time 07:21 PM" in the top right corner
- "Powered by Zephyr" at the bottom left
- "Author: Stellar" at the bottom right  
- Four small squares at the top left

### Console Output:
```
[00:00:00.123,456] <inf> epaper_simple: Display device ready.
[00:00:00.234,567] <inf> epaper_simple: Display blanking is off. Screen should be cleared by full refresh.
[00:00:00.345,678] <inf> epaper_simple: Display width: 800, height: 480
```

## Display Characteristics
- **Resolution**: 800x480 pixels
- **Color Depth**: 1-bit (monochrome)
- **Refresh Rate**: Very slow (several seconds per full update)
- **Memory**: ~48KB buffer for display contents
- **Controller**: UC8179 (compatible with GoodDisplay GDEW075T7)

## LVGL Configuration
The sample uses several LVGL features:
- Multiple Montserrat font sizes (12, 14, 16, 18, 24)
- Label widgets for text display
- Container objects for layout
- Black text on white background (optimal for e-paper)
- Font compression to save memory

## Power Considerations
- E-paper displays only consume power during updates
- Static images consume no power to maintain
- Full screen refresh takes several seconds
- Partial updates may be faster but not used in this sample

## Customization
To modify the display content:
1. Edit `src/main.c` to change text, positions, or add new elements
2. Use LVGL documentation for advanced graphics
3. Modify `prj.conf` to enable additional LVGL features
4. Update font sizes or add custom fonts as needed

## Troubleshooting
- **Display not updating**: Check SPI connections and power supply
- **Garbled display**: Verify SPI timing and pin assignments  
- **Memory issues**: Reduce LVGL buffer size or disable unused features
- **Slow performance**: Normal for e-paper - avoid frequent updates