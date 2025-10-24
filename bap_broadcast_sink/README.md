# BLE Audio Broadcast Sink (Receiver)

Companion receiver sample for the `bap_dmic` wireless microphone system. Receives BLE Audio broadcasts and outputs to serial console or USB audio.

## Quick Start for Wireless Microphone System

This sample works with the `bap_dmic` transmitter to create a complete wireless microphone:

**XIAO nRF54L15** (bap_dmic) → **nRF52840 Dongle** (this sample) → **Windows Speakers**

## Building for nRF52840 Dongle

### Using Existing Build (Fastest)

If you already have a build directory:

```powershell
cd M:\nRF54L15\bap_broadcast_sink
cmake --build build
west flash
```

### Clean Build

```powershell
cd M:\nRF54L15\bap_broadcast_sink
west build --pristine -b nrf52840dongle_nrf52840 -- -DEXTRA_CONF_FILE=overlay-bt_ll_sw_split.conf
west flash
```

## Configuration Modes

### Serial Console Mode (Default - Debug)

**Config:** `boards/nrf52840dongle_nrf52840.conf`
```conf
CONFIG_USE_USB_AUDIO_OUTPUT=n
CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=y
CONFIG_UART_CONSOLE=y
```

**Use case:** See debug output showing audio reception statistics

**Serial Output Example:**
```
[00:16:14.398] <inf> stream_rx: Incoming audio on stream 0x20005ef4
Valid 17483 | Error 0 | Loss 4 | Dup TS 0 | Dup PSN 0
```

### USB Audio Mode (Playback)

**Config:** Edit `boards/nrf52840dongle_nrf52840.conf`:
```conf
CONFIG_USE_USB_AUDIO_OUTPUT=y
CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=n
CONFIG_UART_CONSOLE=n
```

**Use case:** Hear audio through Windows speakers

**After flashing:**
1. Dongle appears as "USB Audio Device" in Windows
2. Select as playback device
3. Audio from XIAO microphone plays through speakers

## Switching Modes

To switch between serial console and USB audio:

1. **Edit** `boards/nrf52840dongle_nrf52840.conf`
2. **Change** the CONFIG values as shown above
3. **Rebuild** using `cmake --build build`
4. **Flash** using `west flash`

> **Note:** Cannot use both modes simultaneously - USB Audio and Serial Console share the same USB interface

## Flashing Instructions

### Using nrfjprog (Recommended)

```powershell
west flash
```

The dongle will:
- Enter bootloader mode (red LED)
- Flash firmware
- Reset and start running

### Manual Bootloader Mode

If flashing fails:
1. Press the RESET button on the dongle
2. Wait for red LED
3. Run `west flash` within 10 seconds

## Operation

1. **Power on** the dongle (plug into USB)
2. **Wait** for scanning to start (~2 seconds)
3. **Power on** the XIAO transmitter (bap_dmic)
4. **Dongle automatically discovers** the broadcast
5. **Synchronizes** to the audio stream
6. **Outputs audio** (via serial logs or USB audio depending on config)

### LED Indicators

- **Red LED**: Bootloader mode
- **Blinking**: Normal operation (if programmed to blink)

## Monitoring Reception

### Serial Console Mode

Connect to the dongle's serial port (COM7 or similar) to see:

```
Scanning for broadcast sources...
Found broadcast source: XIAO Audio
Syncing to periodic advertising...
PA synced!
Received BASE:
  - Codec: LC3
  - Sample Rate: 16000 Hz
  - Frame Duration: 10 ms
Creating BIG sync...
BIG synced! Receiving audio...
Stream 0x20005ef4: Valid 1000 packets
```

### USB Audio Mode

No serial output available. Monitor by:
- **Windows Sound Settings**: Shows "USB Audio Device" active
- **Volume Meter**: Should respond to audio from XIAO microphone
- **Playback**: Hear audio through selected output device

## Performance

Typical operation shows excellent reliability:
- **Valid Packets**: 17,483 received
- **Packet Loss**: 4 out of 17,487 (99.98% success)
- **Errors**: 0
- **Duplicates**: 0

## Troubleshooting

### Dongle Not Scanning

**Problem:** No broadcast sources found  
**Solution:**
- Verify XIAO transmitter is running
- Check both devices are powered
- Try resetting both devices
- Ensure BLE antenna is not blocked

### No Audio in USB Mode

**Problem:** USB Audio Device shows but no sound  
**Solution:**
- Check Windows sound settings
- Verify dongle is selected as playback device
- Confirm XIAO is transmitting (check its serial console)
- Try adjusting volume levels

### Flashing Fails

**Problem:** "Cannot connect to target"  
**Solution:**
- Press RESET button on dongle
- Wait for red LED
- Run `west flash` immediately
- Try different USB port
- Check USB cable is data-capable

### High Packet Loss

**Problem:** Many lost/error packets in serial console  
**Solution:**
- Move devices closer together
- Remove obstacles between devices
- Check for RF interference
- Verify XIAO is not overloaded (check its serial output)

## Configuration Files

- `boards/nrf52840dongle_nrf52840.conf` - **Edit this to switch modes**
- `prj.conf` - Main project configuration
- `overlay-bt_ll_sw_split.conf` - Required for BLE Audio on nRF52840

## Related Samples

- `bap_dmic` - Transmitter sample (XIAO nRF54L15 with microphone)
- Nordic BLE Audio samples in nRF Connect SDK

## Complete System Documentation

See `bap_dmic/README.md` for complete wireless microphone system documentation.

## License

This project is licensed under the Apache 2.0 License.
