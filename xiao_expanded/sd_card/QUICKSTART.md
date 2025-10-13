# SD Card + ZMS Sample - Quick Start Guide

## What This Sample Does

This project demonstrates **DUAL STORAGE** on the XIAO nRF54L15:

### 1. Internal Storage (ZMS in RRAM)
- **36KB** partition at `0x15c000` in RRAM
- Stores: Boot count, SD access count, configuration
- Survives power cycles and resets

### 2. External Storage (SD Card)
- MicroSD card via SPI interface on expansion base
- Stores: Files, logs, user data
- FAT32/exFAT filesystem

## Hardware Setup

1. Insert XIAO nRF54L15 into expansion base
2. Insert microSD card (formatted as FAT32)
3. Connect USB for power and serial output

## Pin Connections (Automatic via Expansion Base)

| Function | XIAO Pin | GPIO | Description |
|----------|----------|------|-------------|
| SPI SCK  | D8       | P2.1 | SPI Clock |
| SPI MISO | D9       | P2.4 | SPI Master In |
| SPI MOSI | D10      | P2.2 | SPI Master Out |
| SD CS    | D2       | P1.6 | SD Card Chip Select |

## Build and Flash

```bash
cd xiao_expanded/sd_card
west build -b xiao_nrf54l15/nrf54l15/cpuapp -p
west flash
```

## Expected Output (Success)

```
========================================
XIAO nRF54L15 SD Card + ZMS Sample
========================================
Initializing ZMS...
ZMS mounted successfully at offset 0x15c000
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

## Common Issues

### "SD card init ERROR!"
→ **SD card not inserted** or not formatted as FAT32

### "ZMS sector size too large"  
→ Fixed! Now uses 8 sectors × 4KB = 32KB (fits in 36KB partition)

### "Stack overflow"
→ Fixed! `CONFIG_MAIN_STACK_SIZE=4096`

## Files Structure

```
sd_card/
├── CMakeLists.txt              # Build configuration
├── prj.conf                    # Kconfig options
├── sample.yaml                 # Sample metadata
├── boards/
│   └── xiao_nrf54l15_nrf54l15_cpuapp.overlay  # Hardware config
├── src/
│   └── main.c                  # Main application
└── README_NEW.md              # Full documentation
```

## Key Differences from nRF52840

| Feature | nRF52840 | nRF54L15 |
|---------|----------|----------|
| Internal Storage | NVS (Flash) | ZMS (RRAM) |
| Storage API | `nvs_*()` | `zms_*()` |
| Memory Type | Flash | RRAM |
| SD Card CS Pin | D2 | D2 (same) |
| SPI Pins | D8/D9/D10 | D8/D9/D10 (same) |

## Next Steps

1. Verify SD card is inserted
2. Flash the sample
3. Open serial monitor (115200 baud)
4. Check for successful initialization
5. Modify `main.c` to add your own ZMS settings or SD card operations

## Support

- Full docs: `README_NEW.md`
- Board files: `m:\nRF54L15\zephyr\boards\arm\xiao_nrf54l15\`
- Zephyr ZMS docs: https://docs.zephyrproject.org/latest/services/storage/zms/
