# SD Card Sample for XIAO nRF54L15

This sample demonstrates SD card file operations (read/write) on the Seeed XIAO nRF54L15 with the expansion base using hardware SPI20.

## Hardware Requirements

- Seeed Studio XIAO nRF54L15
- XIAO Expansion Base with SD card slot
- Micro SD card (FAT32 formatted recommended)

## SPI Configuration

This sample uses **hardware SPI20** peripheral with SDHC driver for SD card access.

### Why SPI20 Instead of SPI00?

The XIAO nRF54L15's default SPI00 (mapped to hardware SPIM00) is a high-speed SPI peripheral with a **minimum frequency of 2 MHz**. SD cards require initialization at 400 kHz, which SPIM00 cannot provide.

**Solution**: We use SPI20 which supports the full frequency range needed for SD card initialization (400 kHz) and high-speed operation (up to 24 MHz).

### Pin Configuration

The hardware SPI20 uses the expansion base SD card slot pins:

| Function | XIAO Pin | GPIO Pin | Description |
|----------|----------|----------|-------------|
| SCK      | D8       | P2.1     | SPI Clock   |
| MOSI     | D10      | P2.2     | Data Out    |
| MISO     | D9       | P2.4     | Data In     |
| CS       | D2       | P1.6     | Chip Select |

### UART Configuration

- **UART21**: Used for console, shell, and RPC (hardware pins D6/D7)
  - TX: P2.8 (D6)
  - RX: P2.7 (D7)
  - Baud rate: 115200
- **UART20**: Disabled (conflicts with expansion base usage)

## Device Tree Configuration

See `boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay` for the complete configuration:

```dts
&pinctrl {
    spi20_default: spi20_default {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 2, 1)>,
                    <NRF_PSEL(SPIM_MOSI, 2, 2)>,
                    <NRF_PSEL(SPIM_MISO, 2, 4)>;
        };
    };
};

&spi20 {
    compatible = "nordic,nrf-spim";
    pinctrl-0 = <&spi20_default>;
    pinctrl-1 = <&spi20_sleep>;
    status = "okay";
    
    cs-gpios = <&gpio1 6 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
    
    sdhc0: sdhc@0 {
        compatible = "zephyr,sdhc-spi-slot";
        spi-max-frequency = <24000000>;
        // ...
    };
};
```

## Features

- SD card access via SDHC SPI driver
- FAT filesystem support with long filename support
- Shell interface with filesystem commands
- Mount/unmount SD card filesystem
- Create and write files
- Read file contents
- Directory operations
- Large stack sizes for reliable SD card operations (32KB main stack)
- Power management support

## Building

Build the sample for XIAO nRF54L15:

```bash
cd m:\nRF54L15\xiao_expanded\sd_card_hw
west build -b xiao_nrf54l15/nrf54l15/cpuapp --pristine
```

## Flashing

Flash to the board:

```bash
west flash
```

## Usage

After flashing, connect to the UART21 serial port (D6/D7 on expansion base) at 115200 baud. You'll have access to a shell with filesystem commands.

### Available Shell Commands

The sample includes the filesystem shell (`CONFIG_FILE_SYSTEM_SHELL=y`) with commands like:

- `fs ls <path>` - List directory contents
- `fs cd <path>` - Change directory  
- `fs pwd` - Print working directory
- `fs mkdir <path>` - Create directory
- `fs rm <path>` - Remove file
- `fs cat <file>` - Display file contents

### Example Operations

```bash
# List root directory of SD card
uart:~$ fs ls /SD:

# Create a directory
uart:~$ fs mkdir /SD:/test

# View file contents
uart:~$ fs cat /SD:/some.dat
```

## Expected Output

The sample will:
1. Initialize SPI20 hardware peripheral
2. Mount the SD card via SDHC driver
3. Create test files and directories
4. Provide shell interface for file operations

Monitor the output via UART21 to see the results and use shell commands.

## Configuration

Key configuration options in `prj.conf`:

- `CONFIG_SDHC=y` - SDHC driver support
- `CONFIG_DISK_DRIVER_SDMMC=y` - SD/MMC disk driver
- `CONFIG_FAT_FILESYSTEM_ELM=y` - FAT filesystem with long filename support
- `CONFIG_FILE_SYSTEM_SHELL=y` - Filesystem shell commands
- `CONFIG_MAIN_STACK_SIZE=32000` - Large stack for SD operations
- `CONFIG_SHELL_STACK_SIZE=16000` - Large shell stack
- `CONFIG_HEAP_MEM_POOL_SIZE=16384` - Heap for dynamic allocations

## Troubleshooting

### SD Card Not Detected

- Ensure SD card is properly inserted into expansion base slot
- Check that SD card is formatted as FAT32
- Try a different SD card (some older/slower cards work better)
- Check serial output on UART21 for error messages

### Build Errors

- Ensure you're using nRF Connect SDK v3.1.1 or later
- Make sure XIAO board definition is installed
- Run `west update` to ensure all dependencies are current
- Clean build with `--pristine` flag

### SPI Communication Issues

- The SDHC driver automatically handles speed negotiation (400 kHz init, up to 24 MHz data)
- Some SD cards may not support full 24 MHz speed
- Check for proper electrical connections on expansion base

## Technical Notes

### Memory Allocations

The sample uses increased memory allocations for reliable filesystem operations:
- **Main stack**: 32 KB (for filesystem operations)
- **System workqueue**: 16 KB
- **Heap**: 16 KB  
- **Shell stack**: 16 KB

These large allocations ensure stable operation with FAT filesystem which can require significant stack depth.

### Hardware SPI20 Advantages

Using hardware SPI20 instead of bit-bang provides:
- Higher performance and throughput
- Lower CPU usage
- Reliable DMA transfers
- Full speed range support (400 kHz to 24 MHz)

### Console Routing

All console output, shell interface, and RPC communication is routed through UART21 since UART20 is disabled to avoid conflicts with the expansion base pin usage.
