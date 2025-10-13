# SD Card + ZMS Storage Sample for XIAO nRF54L15

This sample demonstrates how to use **both** SD card storage and ZMS (Zephyr Memory Storage) on the XIAO nRF54L15.

## Dual Storage Architecture

This sample uses TWO independent storage systems:

### 1. **ZMS (Zephyr Memory Storage)** - Internal Flash
- **Location**: Internal RRAM at address `0x15c000` (36KB partition)
- **Purpose**: Persistent settings, configuration, counters
- **Use case**: Boot count, SD access tracking, device configuration
- **Technology**: RRAM (Resistive RAM) - new in nRF54L15

### 2. **SD Card** - External Storage  
- **Location**: External microSD card via SPI interface
- **Purpose**: File storage, data logging, user files
- **Use case**: Storing large files, logs, user data
- **Technology**: FAT32/exFAT filesystem

## Why ZMS Instead of NVS?

The nRF54L15 uses **RRAM** (Resistive RAM) instead of traditional flash memory. Nordic recommends using **ZMS (Zephyr Memory Storage)** instead of NVS (Non-Volatile Storage) for this new hardware. ZMS is designed to work with all types of non-volatile storage technologies including RRAM.

## Features

- **SD Card Access**: Read/write files using FAT filesystem (with exFAT support)
- **ZMS Storage**: Persistent key-value storage in RRAM
- **File Operations**: Create, read, list files and directories
- **Persistent Counters**: Boot count and SD access count stored in ZMS
- **Error Handling**: Comprehensive error messages and diagnostics

## Hardware Requirements

- XIAO nRF54L15 development board
- XIAO Expansion Base with SD card slot
- Micro SD card (formatted as FAT32 or exFAT)
- nRF Connect SDK v3.1.0 or later

## Hardware Connections

The SD card is connected via the XIAO expansion base using SPI:

| SD Card Signal | XIAO Pin | Description |
|----------------|----------|-------------|
| MOSI           | D10      | xiao_spi MOSI |
| MISO           | D9       | xiao_spi MISO |
| SCK            | D8       | xiao_spi SCK  |
| CS             | D2       | GPIO Chip Select |

## Building and Flashing

### Using west (recommended)

```bash
cd xiao_expanded/sd_card
west build -b xiao_nrf54l15/nrf54l15/cpuapp -p
west flash
```

### Using nRF Connect for VS Code

1. Open the `xiao_expanded/sd_card` folder
2. Select board: `xiao_nrf54l15/nrf54l15/cpuapp`
3. Click "Build"
4. Click "Flash"

## What the Sample Does

1. **Initializes ZMS**
   - Mounts ZMS filesystem in RRAM
   - Reads/updates boot counter
   - Stores configuration data

2. **Initializes SD Card**
   - Detects and initializes SD card
   - Reports card size and sector information
   - Updates SD access counter in ZMS

3. **Mounts Filesystem**
   - Mounts FAT filesystem from SD card
   - Tests unmount/remount functionality

4. **File Operations**
   - Lists root directory contents
   - Creates sample files and directories (if enabled)
   - Writes a test file with timestamp

5. **Clean Shutdown**
   - Properly unmounts SD card
   - Persists all ZMS data

## Expected Output

```
========================================
XIAO nRF54L15 SD Card + ZMS Sample
========================================
Initializing ZMS...
ZMS mounted successfully at offset 0xfc000
Boot count: 1
Updated boot count to: 2
Stored config version: 100
Initializing SD card...
SD Card - Block count: 15523840
SD Card - Sector size: 512 bytes
SD Card - Total size: 7580 MB
SD card accessed 1 times
Mounting SD card filesystem...
SD card mounted at /SD:
Testing unmount/remount...
Unmount/remount test successful

Listing dir /SD: ...
[FILE] test.txt (size = 27)
Created test file: /SD:/test.txt
SD card unmounted
========================================
Sample completed. System will idle.
========================================
```

## Configuration Options

### In `prj.conf`:

- **Enable/Disable ZMS**: Comment out ZMS-related configs if not needed
- **Enable/Disable sample file creation**: `CONFIG_FS_SAMPLE_CREATE_SOME_ENTRIES`
- **Enable shell for interactive filesystem**: Uncomment shell configs

### Flash Partition Configuration

The storage partition for ZMS is defined in the overlay file:
- Location: `0xfc000` (end of RRAM)
- Size: `16KB` (0x4000 bytes)
- Sector size: `4KB` (RRAM page size)

## Troubleshooting

### SD Card Not Detected

**Error**: `spi_nrfx_spim: Failed to initialize nrfx driver` or `sdhc_spi: Card SCLK init sequence failed`

**Solutions**:
1. **Check SD card is inserted** - This is the most common issue!
2. **Format SD card** as FAT32 (for cards ≤32GB) or exFAT (for larger cards)
3. **Verify expansion base** - Make sure XIAO is properly seated in expansion base
4. **Check connections**:
   - D8 (P2.1) = SPI SCK
   - D9 (P2.4) = SPI MISO  
   - D10 (P2.2) = SPI MOSI
   - D2 (P1.6) = CS (Chip Select)
5. **Try different SD card** - Some cards may be incompatible
6. **Reduce SPI frequency** - Edit overlay: `spi-max-frequency = <8000000>;` (8MHz instead of 24MHz)

### ZMS Initialization Failed

**Error**: `ZMS sector size too large`

**Solutions**:
1. Verify storage partition size in device tree (should be 36KB at 0x15c000)
2. Check `ZMS_SECTOR_COUNT` is set to 8 (8 × 4KB = 32KB)
3. Ensure `CONFIG_MPU_ALLOW_FLASH_WRITE=y` is set in prj.conf
4. Verify `CONFIG_FLASH=y` is enabled

### Stack Overflow

**Error**: `Stack overflow (context area not valid)`

**Solutions**:
1. Increase `CONFIG_MAIN_STACK_SIZE` in prj.conf (current: 4096)
2. Try setting to 8192 if still occurring
3. Check for recursive function calls or large stack allocations

### File System Mount Failed

**Error**: Mount fails after successful SD card init

**Solutions**:
1. Format SD card as FAT32 using a computer
2. Ensure SD card is not write-protected
3. Check logs for specific FAT filesystem error codes
4. Try a freshly formatted card

## API References

- [Zephyr File System API](https://docs.zephyrproject.org/latest/services/storage/file_systems.html)
- [Zephyr Memory Storage (ZMS)](https://docs.zephyrproject.org/latest/services/storage/zms/zms.html)
- [Nordic ZMS Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/device_guides/nrf54l/zms.html)
- [SDHC SPI Driver](https://docs.zephyrproject.org/latest/hardware/peripherals/sdhc.html)

## License

SPDX-License-Identifier: Apache-2.0
