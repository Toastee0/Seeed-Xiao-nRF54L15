# DMIC Audio Recorder Sample

This sample demonstrates how to record audio from the digital microphone (DMIC) on the Seeed XIAO nRF54L15 and stream it over UART to a PC for saving as a WAV file.

## Features
- Digital microphone (PDM) audio capture
- 16 kHz sample rate, 16-bit PCM
- Button-triggered recording
- UART streaming at 921600 baud
- LED indicator during recording
- Python script for PC-side audio capture

## Hardware Requirements
- Seeed XIAO nRF54L15 development board with built-in DMIC
- USB cable for connection to PC
- No additional hardware required (uses built-in microphone)

## Building and Running

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Recording Audio

1. **Flash the firmware** to the XIAO nRF54L15
2. **Connect to PC** via USB (the board will appear as a serial device)
3. **Run the Python recording script**:

### On Windows:
```bash
python scripts/record.py -p COM3 -o recording.wav -b 921600
```

### On Linux:
```bash  
python scripts/record.py -p /dev/ttyACM0 -o recording.wav -b 921600
```

4. **Press the SW0 button** on the board within 20 seconds when prompted
5. **Wait 10 seconds** for recording to complete
6. The **LED will light up** during recording
7. Audio will be saved as `recording.wav`

## Expected Output

### Device Console:
```
[00:00:00.123,456] <inf> mic_capture_sample: Zephyr Audio Streamer Ready.
[00:00:00.123,678] <inf> mic_capture_sample: Press button SW0 to start recording...
[00:00:05.234,567] <inf> mic_capture_sample: Button pressed, starting capture...
[00:00:15.345,678] <inf> mic_capture_sample: Audio capture finished and data queued.
[00:00:15.345,890] <inf> mic_capture_sample: Press button SW0 to start recording again...
```

### Python Script Output:
```
--- Zephyr/Python Audio Recorder ---
  - Serial port: COM3, Baudrate: 921600
  - Output: recording.wav
  - Expected bytes: 320000
------------------------------------
Serial port opened. Please press SW0 on device within 20 seconds...
Synchronized (START packet received). Receiving audio data...
Received 320000 bytes of audio data.
Transfer verified (END packet received).
Saving to 'recording.wav'...
WAV file saved.
```

## Technical Details
- **Sample Rate**: 16 kHz
- **Bit Width**: 16-bit PCM
- **Channels**: Mono (1 channel)
- **Recording Duration**: 10 seconds
- **Data Rate**: ~32 KB/s audio data
- **UART Speed**: 921600 baud for fast transfer
- **Buffer Size**: 100ms chunks (3200 bytes each)

## Script Dependencies
The Python script will automatically install `pyserial` if not available:
```bash
pip install pyserial
```

## Troubleshooting
- **Serial port not found**: Check device manager (Windows) or `ls /dev/tty*` (Linux)
- **Permission denied (Linux)**: Add user to `dialout` group: `sudo usermod -a -G dialout $USER`
- **Timeout waiting for start packet**: Ensure firmware is running and press SW0 button
- **Audio data timeout**: Check UART connection and baud rate
- **Audio quality issues**: Ensure good microphone placement and avoid noise sources

## File Structure
```
zephyr-dmic-recorder-ncs/
├── src/main.c                    # Main application
├── scripts/record.py             # PC recording script
├── boards/                       # Device tree overlay
├── CMakeLists.txt               # Build configuration
├── prj.conf                     # Zephyr configuration
├── sample.yaml                  # Sample metadata
└── README.md                    # This file
```