# BLE Audio Broadcast Source with DMIC Microphone

Complete wireless microphone system using BLE Audio (BAP - Basic Audio Profile) with LC3 codec and PDM microphone input.

## Overview

This sample demonstrates:
- **PDM microphone capture** from the XIAO nRF54L15's onboard DMIC
- **LC3 audio encoding** for efficient wireless transmission
- **BLE Audio broadcast** using BAP Broadcast Source profile
- **Real-time audio streaming** at 16kHz, 16-bit mono

The audio is captured from the PDM20 microphone, encoded using LC3 codec, and broadcast over BLE Audio for reception by compatible devices.

## Hardware Requirements

- **Seeed XIAO nRF54L15 Sense** (transmitter with microphone)
- **nRF52840 Dongle** or compatible BLE Audio receiver (see companion `bap_broadcast_sink` sample)

## Features

- ✅ 10ms audio frames (160 samples @ 16kHz)
- ✅ LC3 codec compression
- ✅ Dual-stream stereo broadcast (both channels carry same mono audio)
- ✅ EasyDMA-based microphone capture
- ✅ Zero-copy buffer management
- ✅ Real-time RMS level monitoring

## Building and Flashing

### Using nRF Connect for VS Code (Recommended)

1. Open this folder in VS Code
2. The sample will automatically appear in nRF Connect extension
3. Click "Add Build Configuration"
4. Select board: `xiao_nrf54l15/nrf54l15/cpuapp`
5. Click "Build"
6. Click "Flash"

### Using Command Line

```powershell
cd M:\nRF54L15\bap_dmic
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Usage

1. **Flash the transmitter** (XIAO nRF54L15) with this sample
2. **Flash the receiver** (nRF52840 dongle) with `bap_broadcast_sink` (see below)
3. Power on both devices
4. The XIAO will start broadcasting audio from its microphone
5. The dongle will automatically discover and connect to the broadcast
6. Speak into the XIAO's microphone to hear audio on the receiver

### Serial Console Output

Connect to the XIAO's serial port (COM8 or similar) to monitor:
- DMIC initialization status
- BLE Audio stream status
- RMS audio levels (updates every 5 seconds)
- ISO packet transmission statistics

Example output:
```
DMIC microphone initialized and thread started
Creating broadcast source
Stream 0x20000d00 started with BIG_Handle 0 and BIS_Number 1
Stream 0x20001760 started with BIG_Handle 0 and BIS_Number 2
Broadcast source started
DMIC RMS level: 73 (speak into mic to see it change)
Stream 0x20000d00: Sent 1000 total ISO packets
```

## Companion Receiver Sample

This sample requires a receiver running the `bap_broadcast_sink` sample. See the [Receiver Setup Guide](#setting-up-the-receiver) below.

## Technical Details

### Audio Pipeline

```
PDM Microphone → EasyDMA → DMIC Driver → Copy Buffer → LC3 Encoder → BLE Audio Broadcast
   (PDM20)      (320 bytes)   (10ms)     (160 samples)   (compressed)    (ISO packets)
```

### Memory Configuration

- **DMIC Buffer Pool**: 4 blocks × 320 bytes (10ms @ 16kHz)
- **RAM Usage**: ~95KB total
- **Flash Usage**: ~271KB

### Buffer Management

The sample uses a **copy-and-free pattern** to avoid blocking the PDM EasyDMA:

1. DMIC driver allocates buffer from mem_slab
2. PDM peripheral fills buffer via DMA
3. Application thread receives buffer pointer
4. **Immediately copies** data to static buffer
5. **Immediately frees** DMA buffer back to mem_slab
6. LC3 encoder reads from static buffer

This ensures the PDM peripheral always has free buffers available for continuous capture.

### Configuration Files

- `prj.conf` - Main project configuration (BLE Audio, LC3 codec, DMIC)
- `boards/xiao_nrf54l15_nrf54l15_cpuapp.conf` - Board-specific settings
- `boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay` - Device tree overlay (PDM20 configuration)

## Setting Up the Receiver

To hear the audio, you need to set up the companion `bap_broadcast_sink` sample on an nRF52840 dongle.

### Option 1: Serial Console Mode (Debug)

Use this mode to see debug output and verify the audio is being received.

**Receiver Configuration:**
```
CONFIG_USE_USB_AUDIO_OUTPUT=n
CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=y
CONFIG_UART_CONSOLE=y
```

**Steps:**
1. Navigate to `bap_broadcast_sink` sample
2. Build for nRF52840 dongle:
   ```powershell
   cd M:\nRF54L15\bap_broadcast_sink
   west build -b nrf52840dongle/nrf52840
   west flash
   ```
3. Connect to dongle's serial port to see audio reception logs

### Option 2: USB Audio Mode (Playback)

Use this mode to hear the audio through your computer's speakers.

**Receiver Configuration:**
```
CONFIG_USE_USB_AUDIO_OUTPUT=y
CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=n
CONFIG_UART_CONSOLE=n
```

**Steps:**
1. Edit `bap_broadcast_sink/boards/nrf52840dongle_nrf52840.conf`:
   - Change `CONFIG_USE_USB_AUDIO_OUTPUT=n` to `CONFIG_USE_USB_AUDIO_OUTPUT=y`
   - Change `CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=y` to `CONFIG_BOARD_SERIAL_BACKEND_CDC_ACM=n`
   - Change `CONFIG_UART_CONSOLE=y` to `CONFIG_UART_CONSOLE=n`

2. Rebuild and flash:
   ```powershell
   cd M:\nRF54L15\bap_broadcast_sink
   cmake --build build
   west flash
   ```

3. Plug dongle into PC - Windows will detect it as "USB Audio Device"
4. Set it as your default playback device (or select in audio settings)
5. Speak into the XIAO's microphone - you'll hear it through your speakers!

### Switching Between Modes

To switch back to debug mode, reverse the config changes and rebuild. Only one mode can be active at a time (USB Audio OR Serial Console, not both).

## Troubleshooting

### No Audio Being Captured

**Problem:** RMS level stays at 0 or very low constant value  
**Solution:** Verify PDM microphone is enabled and working:
- Check device tree overlay has `&pdm20 { status = "okay"; };`
- Ensure board pinctrl is using defaults (pins P0.12 CLK, P0.13 DIN)
- Try speaking louder or tapping near the microphone

### MPU Fault on Startup

**Problem:** System crashes with MPU fault  
**Solution:** This was caused by incorrect buffer management. The current version:
- Uses 10ms blocks (320 bytes)
- Immediately frees DMA buffers after copying
- Has 4 buffer blocks for double-buffering
- Should not exhibit this issue

### Receiver Not Connecting

**Problem:** Dongle doesn't find or connect to broadcast  
**Solution:**
- Verify both devices are powered and flashed correctly
- Check serial output on both devices
- Ensure receiver is in scanning mode
- Try power-cycling both devices

### Audio Quality Issues

**Problem:** Audio sounds distorted or choppy  
**Solution:**
- Check RMS levels - should vary between 50-1000 for normal speech
- Verify LC3 encoding is working (check for "LC3 encoder failed" messages)
- Ensure ISO packet transmission is continuous (check "Sent X total ISO packets")
- Verify receiver is not reporting packet loss

## Performance Metrics

Typical operation (from serial console):

- **Audio Capture**: 100 blocks/second (10ms per block)
- **BLE Transmission**: 100 ISO packets/second per stream (200 total)
- **Packet Success Rate**: >99.9% (typically 0-4 lost packets per 17,000)
- **CPU Usage**: Moderate (BLE stack + LC3 encoding + DMIC capture)

## Key Code Sections

### DMIC Thread (Producer)
```c
// Captures audio from microphone
ret = dmic_read(dmic_dev, 0, &buffer, &size, SYS_FOREVER_MS);
memcpy(latest_audio_buffer, buffer, size);  // Copy to static buffer
k_mem_slab_free(&dmic_mem_slab, buffer);    // Free DMA buffer immediately
```

### LC3 Encoder Thread (Consumer)
```c
// Reads from shared buffer and encodes
k_mutex_lock(&audio_data_mutex, K_FOREVER);
memcpy(send_pcm_data, latest_audio_buffer, samples_to_copy * sizeof(int16_t));
k_mutex_unlock(&audio_data_mutex);
lc3_encode(encoder, LC3_PCM_FORMAT_S16, send_pcm_data, ...);
bt_bap_stream_send(stream, buf, seq_num++);
```

## Related Samples

- `bap_broadcast_sink` - Companion receiver sample (nRF52840 dongle)
- `dmic` - Simple DMIC capture with LED feedback
- `dmic_easydma` - Standalone DMIC EasyDMA demonstration

## References

- [Nordic BLE Audio Documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/bluetooth/bluetooth-audio.html)
- [LC3 Codec Specification](https://www.bluetooth.com/specifications/lc3/)
- [PDM Microphone (PDM20) Documentation](https://docs.nordicsemi.com/bundle/ps_nrf54l15/page/pdm.html)
- [EasyDMA Documentation](https://docs.nordicsemi.com/bundle/ps_nrf54l15/page/easydma.html)

## License

This project is licensed under the Apache 2.0 License.
