#!/usr/bin/env python3
"""
Real-time Speech Transcription for BLE Audio Wireless Microphone
Captures audio from nRF52840 dongle (USB Audio Device) and transcribes using whisper.cpp

Requirements:
    pip install numpy sounddevice

Usage:
    python transcribe.py [--model base] [--device "USB Audio"]
"""

import argparse
import subprocess
import sys
import os
from pathlib import Path

def find_whisper_exe():
    """Find the whisper executable"""
    # Get script directory
    script_dir = Path(__file__).parent
    
    # Prefer main.exe for real-time capture
    exe_names = ["main.exe", "whisper-cli.exe"]
    
    possible_dirs = [
        script_dir / "whisper.cpp/build/bin/Release",
        script_dir / "whisper.cpp/build/bin",
        script_dir / "whisper.cpp/build/Release/bin",
        script_dir / "whisper.cpp",
    ]
    
    for dir_path in possible_dirs:
        for exe_name in exe_names:
            full_path = dir_path / exe_name
            if full_path.exists():
                return full_path.absolute()
    
    return None

def find_model(model_name):
    """Find the model file"""
    # Get script directory
    script_dir = Path(__file__).parent
    
    model_path = script_dir / f"whisper.cpp/models/ggml-{model_name}.bin"
    
    if model_path.exists():
        return model_path.absolute()
    
    return None

def list_audio_devices():
    """List available audio devices using sounddevice"""
    try:
        import sounddevice as sd
        print("\n=== Available Audio Devices ===")
        devices = sd.query_devices()
        for i, device in enumerate(devices):
            if device['max_input_channels'] > 0:
                print(f"  [{i}] {device['name']} (Input)")
                print(f"      Channels: {device['max_input_channels']}, "
                      f"Sample Rate: {device['default_samplerate']}")
        print("=" * 40)
    except ImportError:
        print("⚠ sounddevice not installed - can't list devices")
        print("  Install with: pip install sounddevice")

def main():
    parser = argparse.ArgumentParser(
        description="Real-time transcription from BLE Audio wireless microphone"
    )
    parser.add_argument(
        "--model",
        choices=["tiny", "base", "small", "medium", "large"],
        default="base",
        help="Whisper model to use (default: base)"
    )
    parser.add_argument(
        "--step",
        type=int,
        default=500,
        help="Audio chunk size in ms (default: 500)"
    )
    parser.add_argument(
        "--length",
        type=int,
        default=5000,
        help="Context length in ms (default: 5000)"
    )
    parser.add_argument(
        "--threads",
        type=int,
        default=8,
        help="Number of CPU threads (default: 8)"
    )
    parser.add_argument(
        "--no-gpu",
        action="store_true",
        help="Disable GPU acceleration"
    )
    parser.add_argument(
        "--list-devices",
        action="store_true",
        help="List available audio input devices"
    )
    
    args = parser.parse_args()
    
    # List devices if requested
    if args.list_devices:
        list_audio_devices()
        return 0
    
    # Find whisper executable
    whisper_exe = find_whisper_exe()
    if not whisper_exe:
        print("✗ whisper.cpp stream.exe not found", file=sys.stderr)
        print("  Run setup.ps1 first to build whisper.cpp", file=sys.stderr)
        return 1
    
    print(f"✓ Found whisper.cpp: {whisper_exe}")
    exe_name = whisper_exe.name
    
    # Find model
    model_path = find_model(args.model)
    if not model_path:
        print(f"✗ Model not found: ggml-{args.model}.bin", file=sys.stderr)
        print(f"  Run setup.ps1 to download the model:", file=sys.stderr)
        print(f"    .\\setup.ps1 -Model {args.model}", file=sys.stderr)
        return 1
    
    print(f"✓ Found model: {model_path}")
    
    # Build command for real-time audio capture
    cmd = [
        str(whisper_exe),
        "-m", str(model_path),
        "-t", str(args.threads),
        "-c", "0",  # Capture from default audio device (microphone)
        "-l", "en",  # Language (English)
        "--print-colors",  # Colorize output
    ]
    
    # Add GPU flag if not disabled (--no-gpu flag disables GPU)
    if args.no_gpu:
        cmd.append("--no-gpu")
        print("ℹ GPU acceleration disabled")
    else:
        print("✓ GPU acceleration enabled (default)")
    
    print("")
    print("=" * 60)
    print(f"Starting transcription with {args.model} model")
    print("=" * 60)
    print("")
    print("Instructions:")
    print("  1. Ensure nRF52840 dongle is connected (USB Audio mode)")
    print("  2. Ensure XIAO nRF54L15 is running and transmitting")
    print("  3. Speak into the XIAO microphone")
    print("  4. Press Ctrl+C to stop")
    print("")
    print("-" * 60)
    print("")
    
    try:
        # Run whisper stream from the whisper.cpp directory
        whisper_dir = whisper_exe.parent.parent.parent.parent  # Go up from bin/Release/stream.exe to whisper.cpp
        result = subprocess.run(cmd, cwd=str(whisper_dir))
        return result.returncode
    except KeyboardInterrupt:
        print("\n\nStopped transcription.")
        return 0
    except Exception as e:
        print(f"\n✗ Error: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    sys.exit(main())
