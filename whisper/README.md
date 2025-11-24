# Whisper Speech-to-Text Server

Real-time speech transcription using whisper.cpp with GPU acceleration, capturing audio from the BLE Audio wireless microphone system.

## Overview

This setup captures audio from the nRF52840 dongle (configured as USB Audio Device) and transcribes speech to text using OpenAI's Whisper model with GPU acceleration.

**Audio Pipeline:**
```
XIAO Microphone → BLE Audio → nRF52840 Dongle → Windows USB Audio → Whisper.cpp → Text Output
```

## Prerequisites

- **Windows 10/11** with CUDA-capable GPU
- **CUDA Toolkit** (for GPU acceleration)
- **CMake** (3.21 or higher)
- **Visual Studio 2019/2022** (with C++ build tools)
- **Git**
- **Python 3.8+** (for helper scripts)

## Installation

### 1. Clone whisper.cpp

```powershell
cd M:\nRF54L15\whisper
git clone https://github.com/ggerganov/whisper.cpp.git
cd whisper.cpp
```

### 2. Build with CUDA Support

```powershell
# Create build directory
mkdir build
cd build

# Configure with CUDA
cmake .. -DGGML_CUDA=ON -DCMAKE_BUILD_TYPE=Release

# Build (this will take a few minutes)
cmake --build . --config Release
```

### 3. Download Whisper Model

Download a model (larger = more accurate but slower):

**Tiny (fastest, ~75MB):**
```powershell
cd M:\nRF54L15\whisper\whisper.cpp\models
.\download-ggml-model.cmd tiny
```

**Base (balanced, ~140MB):**
```powershell
.\download-ggml-model.cmd base
```

**Small (good accuracy, ~460MB):**
```powershell
.\download-ggml-model.cmd small
```

**Medium (high accuracy, ~1.5GB):**
```powershell
.\download-ggml-model.cmd medium
```

## Usage

### Step 1: Start the Wireless Microphone

1. **Flash XIAO nRF54L15** with `bap_dmic` sample
2. **Flash nRF52840 dongle** with `bap_broadcast_sink` in **USB Audio mode**
   - Edit `bap_broadcast_sink/boards/nrf52840dongle_nrf52840.conf`:
   - Set `CONFIG_USE_USB_AUDIO_OUTPUT=y`
   - Rebuild and flash
3. **Connect dongle** to PC
4. **Verify** Windows detects "USB Audio Device"

### Step 2: Verify Audio Device

Check Windows Sound Settings:
- Open "Sound Settings"
- Find "USB Audio Device" in input devices
- Set as default recording device (optional)
- Test by speaking - meter should respond

### Step 3: Run Whisper Stream

**Real-time transcription:**
```powershell
cd M:\nRF54L15\whisper\whisper.cpp
.\build\bin\Release\stream.exe -m models\ggml-base.bin -t 8 --step 500 --length 5000
```

**Parameters:**
- `-m models\ggml-base.bin` - Model to use (base recommended for balance)
- `-t 8` - Number of CPU threads
- `--step 500` - Audio chunk size (500ms)
- `--length 5000` - Context length (5 seconds)

**With GPU acceleration:**
```powershell
.\build\bin\Release\stream.exe -m models\ggml-base.bin -t 8 --step 500 --length 5000 -ng 1
```

### Step 4: Transcribe

Speak into the XIAO microphone and watch the terminal for transcriptions!

## Advanced Usage

### File-based Transcription

If you want to transcribe recorded audio files:

```powershell
.\build\bin\Release\main.exe -m models\ggml-base.bin -f audio.wav
```

### Continuous Monitoring

For continuous background transcription, use the `stream` example with logging:

```powershell
.\build\bin\Release\stream.exe -m models\ggml-base.bin -t 8 --step 500 --length 5000 > transcription.txt
```

### Python Integration

Create a Python script to capture and process:

```python
import whisper
import sounddevice as sd
import numpy as np

# Load model
model = whisper.load_model("base")

# Capture audio
def callback(indata, frames, time, status):
    # Process with whisper
    audio = np.squeeze(indata)
    result = model.transcribe(audio)
    print(result["text"])

# Start capture from USB Audio Device
with sd.InputStream(callback=callback, channels=1, samplerate=16000):
    input("Press Enter to stop...")
```

## Performance Tips

### GPU Selection

If you have multiple GPUs:
```powershell
set CUDA_VISIBLE_DEVICES=0  # Use first GPU
.\build\bin\Release\stream.exe -m models\ggml-base.bin -ng 1
```

### Model Selection Trade-offs

| Model  | Size   | Speed      | Accuracy | Recommended Use        |
|--------|--------|------------|----------|------------------------|
| Tiny   | 75MB   | Fastest    | Good     | Real-time, low latency |
| Base   | 140MB  | Fast       | Better   | **Recommended default**|
| Small  | 460MB  | Moderate   | High     | Offline transcription  |
| Medium | 1.5GB  | Slow       | Highest  | Maximum accuracy       |

### Latency Reduction

For lowest latency:
- Use `tiny` model
- Reduce `--length` to 3000 (3 seconds)
- Reduce `--step` to 250 (250ms chunks)
- Enable GPU acceleration (`-ng 1`)

```powershell
.\build\bin\Release\stream.exe -m models\ggml-tiny.bin -t 8 --step 250 --length 3000 -ng 1
```

## Troubleshooting

### No Audio Device Found

**Problem:** whisper.cpp can't find USB Audio Device  
**Solution:**
- Verify dongle is connected and configured for USB Audio mode
- Check Windows Sound Settings shows the device
- Try specifying device explicitly in stream command
- Restart audio service: `net stop audiosrv && net start audiosrv`

### GPU Not Being Used

**Problem:** Transcription is slow, GPU shows 0% usage  
**Solution:**
- Verify CUDA toolkit is installed: `nvcc --version`
- Rebuild with `-DWHISPER_CUBLAS=ON`
- Check GPU is detected: `nvidia-smi`
- Ensure `-ng 1` flag is used when running

### Poor Transcription Quality

**Problem:** Text output is inaccurate  
**Solution:**
- Use larger model (base → small → medium)
- Increase `--length` for more context
- Check audio quality from dongle (test in Windows recorder)
- Speak clearly and reduce background noise
- Adjust microphone position on XIAO

### High Latency

**Problem:** Delay between speaking and transcription  
**Solution:**
- Use smaller model (tiny or base)
- Reduce `--step` and `--length` parameters
- Enable GPU acceleration
- Close other GPU-intensive applications

## Example Output

```
[00:00:02.500 --> 00:00:05.000]   Hello, this is a test of the wireless microphone.
[00:00:05.500 --> 00:00:08.000]   The audio quality is quite good.
[00:00:08.500 --> 00:00:11.500]   Whisper is transcribing in real time using the GPU.
```

## Python Helper Script

Create `capture_and_transcribe.py`:

```python
#!/usr/bin/env python3
"""
Real-time transcription from BLE Audio wireless microphone
"""

import subprocess
import sys

def main():
    model = "base"  # Change to tiny/small/medium as needed
    
    cmd = [
        r"whisper.cpp\build\bin\Release\stream.exe",
        "-m", f"whisper.cpp/models/ggml-{model}.bin",
        "-t", "8",
        "--step", "500",
        "--length", "5000",
        "-ng", "1"  # GPU acceleration
    ]
    
    print(f"Starting transcription with {model} model...")
    print("Speak into the XIAO microphone...")
    print("-" * 50)
    
    try:
        subprocess.run(cmd, cwd=r"M:\nRF54L15\whisper")
    except KeyboardInterrupt:
        print("\nStopped transcription.")

if __name__ == "__main__":
    main()
```

Run with: `python capture_and_transcribe.py`

## Integration with BLE Audio System

This whisper setup completes the full pipeline:

1. **XIAO nRF54L15** (`bap_dmic`) - Captures audio from PDM microphone
2. **nRF52840 Dongle** (`bap_broadcast_sink`) - Receives and outputs as USB audio
3. **Whisper.cpp** (this setup) - Transcribes speech to text

For full system documentation, see:
- `../bap_dmic/README.md` - Transmitter setup
- `../bap_broadcast_sink/README.md` - Receiver setup

## References

- [whisper.cpp GitHub](https://github.com/ggerganov/whisper.cpp)
- [OpenAI Whisper](https://github.com/openai/whisper)
- [CUDA Toolkit Download](https://developer.nvidia.com/cuda-downloads)

## License

whisper.cpp is licensed under the MIT License.
