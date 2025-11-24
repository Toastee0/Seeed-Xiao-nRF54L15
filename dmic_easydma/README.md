# DMIC EasyDMA Sample

A clean, verified implementation of DMIC audio capture using Nordic's EasyDMA for the XIAO nRF54L15, optimized for BLE Audio LC3 codec integration.

## ✅ Verified Against Nordic Documentation

This implementation has been verified against Nordic's official EasyDMA documentation and correctly implements all EasyDMA concepts. See [VERIFICATION.md](VERIFICATION.md) for detailed analysis.

## Features

- ✅ **16kHz, 16-bit, Mono** audio capture (LC3 compatible)
- ✅ **10ms frame size** (160 samples = 320 bytes)
- ✅ **EasyDMA-based capture** - zero-copy from peripheral to RAM
- ✅ **4-buffer DMA pool** for continuous operation
- ✅ **Ring buffer** (8 frames = 80ms) for producer-consumer pattern
- ✅ **Real-time RMS monitoring** to verify audio capture
- ✅ **Performance statistics** (capture/consume rates, overflow detection)

## How It Works

### EasyDMA Flow:
1. **PDM Peripheral** captures audio via EasyDMA → writes directly to `dma_mem_slab` buffers in RAM
2. **Capture Thread** calls `dmic_read()` → gets pointer to DMA-filled buffer (zero-copy)
3. **Ring Buffer** receives copied audio data for processing
4. **Consumer Thread** reads from ring buffer → simulates LC3 encoder
5. **Buffer Recycling** - DMA buffer freed back to mem_slab for reuse

### Memory Layout:
```
┌─ DMA Buffers (mem_slab) ──┐
│  Buffer 0: 320 bytes      │ ← PDM writes via EasyDMA
│  Buffer 1: 320 bytes      │   (no CPU intervention)
│  Buffer 2: 320 bytes      │
│  Buffer 3: 320 bytes      │
└───────────────────────────┘
           │
           │ dmic_read() returns pointer
           ↓
┌─ Application Thread ──────┐
│  Copies to ring buffer    │
│  Frees DMA buffer         │
└───────────────────────────┘
           │
           ↓
┌─ Ring Buffer (2560 B) ────┐
│  Holds 8 frames (80ms)    │ ← LC3 Encoder reads
└───────────────────────────┘
```

## Building

```bash
cd dmic_easydma
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Output Example

```
=== DMIC EasyDMA Sample ===
Sample Rate: 16000 Hz
Frame Size: 160 samples (320 bytes, 10 ms)
DMA Blocks: 4
Ring Buffer: 2560 bytes (8 frames)
Configuring DMIC...
Starting DMIC DMA capture...
DMIC running! Threads will process audio continuously.
Speak into microphone to see RMS levels change.

DMIC capture thread started
Audio consumer thread started (simulating LC3 encoder)
Audio RMS:  1234 (speak into mic to see change)
Captured: 100 blocks, Ring buf: 640/2560 bytes (25.0% full)
Audio RMS:  2567 (speak into mic to see change)
Captured: 200 blocks, Ring buf: 960/2560 bytes (37.5% full)

=== Status ===
Captured: 1000 blocks, Consumed: 998 blocks
Ring buffer overflows: 0
Performance: 99.8% (100% = perfect match)
```

## Key Files

- **src/main.c** - Main implementation with DMA capture and ring buffer
- **EASYDMA_NOTES.md** - Detailed EasyDMA documentation and concepts
- **VERIFICATION.md** - Verification against Nordic's official documentation
- **boards/*.overlay** - Device tree configuration for PDM20 peripheral

## Integration with BLE Audio

This sample is designed to be integrated into the BLE Audio broadcast source:

1. **DMA capture thread** remains the same
2. **Ring buffer** feeds the LC3 encoder instead of consumer thread
3. **Frame size** (10ms) matches LC3 requirements
4. **Sample rate** (16kHz) matches BLE Audio preset
5. **Buffer management** proven to work without overflows

## References

- [Nordic EasyDMA Documentation](https://docs.nordicsemi.com/category/easydma)
- [EASYDMA_NOTES.md](EASYDMA_NOTES.md) - Our detailed notes
- [VERIFICATION.md](VERIFICATION.md) - Implementation verification

## Next Steps

1. ✅ DMIC EasyDMA sample created and verified
2. ⏭️ Integrate this pattern into `bap_broadcast_source`
3. ⏭️ Replace test tones with real DMIC data
4. ⏭️ Test complete wireless microphone pipeline
