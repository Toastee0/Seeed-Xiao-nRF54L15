# SD Card Sample for Xiao Expansion Base

This sample demonstrates SD card file operations (read/write) on the Seeed Xiao nRF54L15 with the expansion base.

## Hardware Requirements

- Seeed Xiao nRF54L15 board
- Seeed Xiao Expansion Base
- MicroSD card (formatted as FAT32)

## Important: SPI Bit-Bang Implementation

This sample uses **software SPI (bit-bang)** instead of hardware SPI for a critical reason:

### Why Bit-Bang SPI?

The nRF54L15's hardware SPI peripheral (SPIM120) has a **minimum frequency of 2MHz**. However, SD cards require slower speeds during initialization (typically 100-400 kHz) for reliable communication. The hardware SPI cannot operate slowly enough for proper SD card initialization.

**Solution**: We use Zephyr's `zephyr,spi-bitbang` driver which implements SPI in software using GPIO pins. This allows us to:
- Control the SPI speed down to the required low frequencies
- Successfully initialize and communicate with SD cards
- Maintain stable file I/O operations

### Pin Configuration

The bit-bang SPI uses the same physical pins as the expansion base SD card slot:
- **CLK (SCK)**: D8 / GPIO2.1
- **MOSI**: D10 / GPIO2.2  
- **MISO**: D9 / GPIO2.4
- **CS**: D2 / GPIO1.6

The hardware SPI peripheral (`&xiao_spi`) is explicitly disabled in the overlay to prevent conflicts.

## Devicetree Configuration

See `boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay` for the complete configuration:

```dts
/ {
    spibb0: spibb0 {
        compatible = "zephyr,spi-bitbang";
        status = "okay";
        // GPIO pin definitions
        
        sdhc0: sdhc@0 {
            compatible = "zephyr,sdhc-spi-slot";
            // SD card configuration as child node
        };
    };
};

&xiao_spi {
    status = "disabled";  // Disable hardware SPI
};
```

## Features

- Mount/unmount SD card filesystem
- Create and write files
- Read file contents
- Directory listing
- FAT32 filesystem support

## Building

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The sample will:
1. Initialize the SPI bit-bang driver
2. Mount the SD card
3. Create a test file and write data
4. Read back the file contents
5. List directory contents

Monitor the output via UART to see the results.

## Troubleshooting

- **Mount fails**: Ensure SD card is formatted as FAT32
- **No SD card detected**: Check that card is fully inserted in expansion base slot
- **File operations fail**: Try a different SD card or reformat as FAT32

## Technical Notes

- The SPI bit-bang driver trades CPU usage for flexibility in timing
- Performance is adequate for SD card operations
- Maximum frequency is set to 24MHz but actual initialization occurs at much lower speeds
- This approach is necessary due to hardware limitations of the nRF54L15's SPI peripheral
