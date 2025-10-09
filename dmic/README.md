# Digital Microphone (DMIC) Audio Sample

This sample demonstrates digital microphone audio capture using the DMIC driver with PDM interface on the Seeed XIAO nRF54L15.

## Features

- PDM (Pulse Density Modulation) digital microphone interface
- Configurable sample rates up to 16kHz
- Both mono and stereo audio capture
- Memory slab allocation for audio buffers
- Configurable PDM clock parameters
- Real-time audio data streaming

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- PDM digital microphone connected to PDM20 interface
- Microphone supporting PDM clock frequency 1-3.5MHz

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The sample will capture audio in two phases - first mono, then stereo:

```
DMIC sample
PCM output rate: 16000, channels: 1
0 - got buffer 0x20001234 of 3200 bytes
1 - got buffer 0x20001567 of 3200 bytes
...
PCM output rate: 16000, channels: 2
0 - got buffer 0x20001890 of 6400 bytes
1 - got buffer 0x20001abc of 6400 bytes
...
Exiting
```

## Configuration

- Audio subsystem and DMIC driver are enabled
- 16-bit sample width at 16kHz sample rate
- Memory slab with 4 blocks for buffering
- PDM clock frequency: 1-3.5MHz
- PDM clock duty cycle: 40-60%
- Block size: 100ms of audio data per block

## Technical Details

- **Sample Rate**: 16kHz
- **Bit Width**: 16-bit
- **Buffer Size**: 100ms blocks
- **Memory Management**: Static memory slab allocation
- **Interface**: PDM20 device node
- **Channels**: Supports both mono (left) and stereo (left+right)
- **Block Count**: 8 blocks total (4 for each configuration)