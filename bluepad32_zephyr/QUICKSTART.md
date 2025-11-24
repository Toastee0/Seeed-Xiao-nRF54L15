# Quick Start Guide - Bluepad32 for Zephyr

## Build and Flash (5 Minutes)

### Step 1: Prerequisites
Ensure you have NCS SDK v3.2.0-preview2 or later installed.

### Step 2: Build
```bash
cd /path/to/bluepad32_zephyr
west build -b xiao_nrf54l15/nrf54l15/cpuapp
```

### Step 3: Flash
```bash
west flash
```

### Step 4: Monitor Logs
```bash
# Linux/macOS
minicom -D /dev/ttyACM0 -b 115200

# Windows
putty -serial COM3 -serspeed 115200
```

## Test with a Controller

### PS4 DualShock 4
1. Hold **Share + PS** buttons until light bar flashes white
2. Watch serial console - should see "Device found" message
3. Board will auto-connect

### Nintendo Switch Pro
1. Hold **Sync** button (top of controller) until LEDs flash
2. Watch for connection message
3. Press buttons to test

### Xbox Controller
1. Hold **Pairing** button (top of controller) until Xbox button flashes
2. Watch for connection
3. Test input

## LED Indicators

- **Slow blink** (1 Hz): Scanning for controllers
- **Solid on**: Controller connected
- **Off**: No activity

## Buttons

- **SW0 (Button 1)**: Toggle scanning on/off
- **SW0 long press**: Clear all bonding data

## Next Steps

1. Check `src/main.c` for data handling examples
2. Modify `prj.conf` to enable/disable parsers
3. Implement your game/robot control logic in the callbacks

## Troubleshooting

**Controller won't connect?**
- Ensure it's in pairing mode (LEDs flashing)
- Try clearing bonds (long press SW0)
- Check serial logs for errors

**Build fails?**
```bash
west build -t pristine  # Clean build
west update             # Update SDK
```

**No serial output?**
- Check correct COM port
- Baud rate should be 115200
- Press reset button on board

## Configuration

Edit `prj.conf`:
```ini
# Disable parsers you don't need to save space
CONFIG_BLUEPAD32_PARSER_DS4=n
CONFIG_BLUEPAD32_PARSER_SWITCH=n

# Reduce max devices for lower RAM usage
CONFIG_BLUEPAD32_MAX_DEVICES=2

# Adjust logging
CONFIG_BLUEPAD32_LOG_LEVEL=3  # 3=INFO, 4=DEBUG
```

## Resources

- Full documentation: `README.md`
- Porting plan: `../xiao_nrf_bluepad32/ZEPHYR_PORTING_PLAN.md`
- NCS docs: https://developer.nordicsemi.com/nRF_Connect_SDK/
