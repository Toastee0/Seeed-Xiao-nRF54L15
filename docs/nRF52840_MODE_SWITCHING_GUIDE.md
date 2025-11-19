# nRF52840 Dongle Mode Switching Guide

## Overview
The nRF52840 dongle can operate in two modes for the BAP broadcast sink project. The mode is controlled by the board-specific configuration file.

## **USB Audio Mode** (for actual audio output)
**Purpose:** Outputs received BLE Audio to Windows as a USB Audio device  
**File to modify:** `M:\nRF54L15\bap_broadcast_sink\boards\nrf52840dongle_nrf52840.conf`

```properties
# Use USB Audio as audio sink
CONFIG_USE_USB_AUDIO_OUTPUT=y
CONFIG_SAMPLE_USBD_PRODUCT="USB Broadcast Sink sample"

# Disable serial console - dedicate USB to audio only
CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=n
CONFIG_UART_CONSOLE=n
CONFIG_CONSOLE=n
CONFIG_SERIAL=n

# Explicitly disable USB CDC ACM
CONFIG_USB_CDC_ACM=n
CONFIG_USB_DEVICE_STACK_NEXT=y
```

**Result:** 
- Shows up as "USB Broadcast Sink sample" USB Audio device in Windows
- Outputs decoded BLE Audio to speakers/headphones
- No debug serial output available

---

## **Debug/Serial Mode** (for debugging and monitoring)
**Purpose:** Provides serial console output for debugging BLE Audio reception  
**File to modify:** `M:\nRF54L15\bap_broadcast_sink\boards\nrf52840dongle_nrf52840.conf`

```properties
# Disable USB Audio for debug mode
CONFIG_USE_USB_AUDIO_OUTPUT=n

# Enable serial console for debug output
CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=y
CONFIG_UART_CONSOLE=y
CONFIG_CONSOLE=y
CONFIG_SERIAL=y

# Enable USB CDC ACM for serial
CONFIG_USB_CDC_ACM=y
CONFIG_USB_DEVICE_STACK_NEXT=y
```

**Result:** 
- Shows up as COM port in Windows Device Manager
- Outputs debug logs via serial terminal
- Can monitor BIS stream reception, sync status, audio data flow
- No audio output (for debugging only)

---

## **CRITICAL NOTES:**

1. **Board config overrides prj.conf**: The board-specific config file (`boards\nrf52840dongle_nrf52840.conf`) **overrides** settings in the main `prj.conf`. This is why changing only `prj.conf` doesn't work.

2. **Rebuild required**: After changing the board config, you must:
   ```
   west build --pristine
   west flash
   ```

3. **USB conflict**: The dongle cannot be both USB Audio and CDC ACM simultaneously - it's one or the other.

## **Switching Procedure:**
1. Edit `M:\nRF54L15\bap_broadcast_sink\boards\nrf52840dongle_nrf52840.conf`
2. Copy the appropriate config section above
3. Build and flash in nRF Connect SDK terminal:
   ```
   cd M:\nRF54L15\bap_broadcast_sink
   west build --pristine
   west flash
   ```
4. Unplug and replug the dongle
5. Check Windows Device Manager for the new device type

## **When to use each mode:**
- **Debug Mode**: When troubleshooting BIS streams, connection issues, or audio pipeline problems
- **USB Audio Mode**: When testing end-to-end audio playback from XIAO microphone to Windows speakers