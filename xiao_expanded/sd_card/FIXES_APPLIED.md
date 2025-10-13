# SD Card + ZMS Sample - Final Fixes Applied

## Issues Fixed

### Issue 1: "ZMS sector size too large"
**Root Cause**: The code was reading the flash page size and overwriting the configured sector size, then checking if it fits.

**Fix**: Removed the line that overwrites `zms.sector_size`. Now it uses the configured value (4KB sectors × 8 = 32KB total, fits in 36KB partition).

**File**: `src/main.c` lines 111-128

```c
/* Before (WRONG): */
zms.sector_size = info.size;  // This overwrote our 4KB with flash page size!

/* After (CORRECT): */
// Just verify our configured size fits, don't overwrite it
uint32_t required_size = zms.sector_size * zms.sector_count;
```

---

### Issue 2: "Failed to initialize nrfx driver: 0bad0004"
**Root Cause**: Missing SPI peripheral compatible string and frequency too high.

**Fix**: Added `compatible = "nordic,nrf-spim"` and reduced frequency from 24MHz to 8MHz.

**File**: `boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay`

```dts
&xiao_spi {
    compatible = "nordic,nrf-spim";  // ADDED THIS
    status = "okay";
    cs-gpios = <&xiao_d 2 GPIO_ACTIVE_LOW>;
    
    sdhc0: sdhc@0 {
        ...
        spi-max-frequency = <8000000>;  // REDUCED FROM 24MHz
    };
};
```

---

## Current Configuration

### ZMS (Internal RRAM Storage)
- **Location**: `0x15c000` (RRAM flash controller)
- **Size**: 8 sectors × 4KB = 32KB (in a 36KB partition)
- **Purpose**: Boot counter, SD access counter, config version
- **Status**: ✅ Should now initialize successfully

### SD Card (External Storage)
- **Interface**: SPI at 8MHz
- **Pins**: 
  - D8 (P2.1) = SCK
  - D9 (P2.4) = MISO  
  - D10 (P2.2) = MOSI
  - D2 (P1.6) = CS
- **Purpose**: File storage, data logging
- **Status**: ✅ Should detect card if inserted and formatted

---

## How to Test

### 1. Build and Flash

**Option A - Use Script (in nRF terminal)**:
```powershell
cd m:\nRF54L15\xiao_expanded\sd_card
.\build_and_flash.ps1
```

**Option B - Manual**:
```bash
cd m:\nRF54L15\xiao_expanded\sd_card
rm -r build -Force
west build -b xiao_nrf54l15/nrf54l15/cpuapp -p
west flash
```

### 2. Expected Output

```
*** Booting nRF Connect SDK v3.1.1 ***
[00:00:00.xxx] <inf> main: ========================================
[00:00:00.xxx] <inf> main: XIAO nRF54L15 SD Card + ZMS Sample
[00:00:00.xxx] <inf> main: ========================================
[00:00:00.xxx] <inf> main: Initializing ZMS...
[00:00:00.xxx] <inf> main: ZMS using 8 sectors of 4096 bytes = 32768 bytes total (partition: 36864 bytes)
[00:00:00.xxx] <inf> main: ZMS mounted successfully at offset 0x15c000
[00:00:00.xxx] <inf> main: Boot count: 1
[00:00:00.xxx] <inf> main: Updated boot count to: 2
[00:00:00.xxx] <inf> main: Stored config version: 100
[00:00:00.xxx] <inf> main: Initializing SD card...
[00:00:00.xxx] <inf> main: SD Card - Block count: 15523840
[00:00:00.xxx] <inf> main: SD Card - Sector size: 512 bytes
[00:00:00.xxx] <inf> main: SD Card - Total size: 7580 MB
[00:00:00.xxx] <inf> main: SD card accessed 1 times
[00:00:00.xxx] <inf> main: Mounting SD card filesystem...
[00:00:00.xxx] <inf> main: SD card mounted at /SD:
[00:00:00.xxx] <inf> main: Testing unmount/remount...
[00:00:00.xxx] <inf> main: Unmount/remount test successful

Listing dir /SD: ...
[FILE] test.txt (size = 27)
[00:00:00.xxx] <inf> main: Created test file: /SD:/test.txt
[00:00:00.xxx] <inf> main: SD card unmounted
[00:00:00.xxx] <inf> main: ========================================
[00:00:00.xxx] <inf> main: Sample completed. System will idle.
[00:00:00.xxx] <inf> main: ========================================
```

### 3. If SD Card Still Fails

1. **No SD card inserted** - Most common issue!
2. **SD card not formatted** - Format as FAT32 on PC
3. **Incompatible SD card** - Try different brand/size
4. **Expansion base connection** - Ensure XIAO is firmly seated

### 4. If ZMS Still Fails

1. Check build output for partition size info
2. Verify `CONFIG_FLASH=y` and `CONFIG_ZMS=y` in prj.conf
3. Check if `storage_partition` exists in device tree

---

## Files Changed

| File | Changes |
|------|---------|
| `src/main.c` | Fixed ZMS sector size logic |
| `boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay` | Added SPI compatible, reduced frequency |
| `prj.conf` | Added stack size, ZMS config |
| `CMakeLists.txt` | Removed incorrect BOARD_ROOT |

---

## Next Steps

1. Test with SD card inserted
2. Verify both ZMS and SD card work
3. Modify for your application needs
4. Consider adding more ZMS settings or SD card file operations

---

## References

- [Zephyr ZMS Documentation](https://docs.zephyrproject.org/latest/services/storage/zms/)
- [Nordic nRF54L15 ZMS Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/device_guides/nrf54l/zms.html)
- [Zephyr SDHC SPI Driver](https://docs.zephyrproject.org/latest/hardware/peripherals/sdhc.html)
