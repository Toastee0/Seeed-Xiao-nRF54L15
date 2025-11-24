# EasyDMA on nRF54L Series

## Overview

**EasyDMA** (Easy Direct Memory Access) allows peripherals to interact directly with system memory (RAM) as a bus manager, **without requiring intervention from the CPU**. This functionality:

- Facilitates efficient data transfers
- Reduces processing demands on the CPU
- Enhances both system performance and power efficiency

**Important**: EasyDMA **cannot access non-volatile memory** (e.g., Flash).

## How It Works

The concept is illustrated below, where:
- **READER** is reading data from **RAMc**
- **WRITER** is writing data to **RAMa** and **RAMb** (each RAM being separate bus subordinates)
- All done through the **Interconnect** and **AHB** buses via **EasyDMA**

```
RAMc ──┐
       ├──> Interconnect ──> AHB ──> Peripheral:
RAMb ──┤                              - READER (EasyDMA) ──┐
       │                              - WRITER (EasyDMA) ──┤──> Peripheral Core
RAMa ──┘                                                   │
                                                           └────┘
```

## RAM Organization on nRF54L15

The RAM is divided into regions (RAM00, RAM01), with each region further divided into sections. This structure supports:

- **Flexible power management** 
- **Power efficiency** through per-section control
- **RAM retention** across various sleep modes (System ON and System OFF)
- **Energy saving** by powering down unused sections during sleep

### Memory Map (nRF54L15)

**RAM01** (0x20040000 - 0x2007FFFF):
- Section 3 (16 kB) - VPR saved context
- Section 3 (16 kB)
- Section 2 (32 kB)
- Section 1 (32 kB)
- Section 0 (32 kB)

**RAM00** (0x20000000 - 0x2003FFFF):
- Section 7 (32 kB)
- ...
- Section 1 (32 kB)
- Section 0 (32 kB)

## EasyDMA Channels and Buffers

Several peripherals on nRF54L Series handle large amounts of data using EasyDMA for efficient data handling. These include:

- **Serial Communication Interfaces** (UART, SPI, TWI)
- **Radio**
- **Cryptographic accelerators**
- **Analog (SAADC)**
- **PWM**
- **PDM (DMIC)**

### Key Features

#### 1. Double-Buffering
For some peripherals, the `.PTR` and `.MAXCNT` registers are **double-buffered in hardware**. This allows software to:
- Update these registers for the next transfer immediately after the current one has started
- Enable **continuous stream of operations without interruption**
- Example: After a STARTED event, prepare the next buffer

#### 2. Array List Mode
Some peripherals support EasyDMA in **Array List mode** for channels with a LIST register. 

**How it works**:
- Data is handled as a linear array in RAM
- Items stored sequentially in memory
- Example: SPIM peripheral uses this for both TX and RX in its EasyDMA
- The hardware **automatically moves to the next buffer** in the list
- **Eliminates** the need to manually configure `.PTR` (by software, involving the CPU) for each memory chunk
- **Very useful for efficient and continuous data streaming**

### EasyDMA Registers

Data transfers are configured using:
- **`.PTR` register**: Specifies the start address of the RAM buffer
- **`.MAXCNT` register**: Defines the maximum number of bytes to transfer

### EasyDMA Events

Once an EasyDMA transfer is complete, the CPU can read the **`.AMOUNT` register** to determine the number of bytes successfully transferred.

**Events that indicate EasyDMA completion**:
- `END`
- `STOPPED`
- `READY`
- Specific DMA-related events (e.g., `DMA.RX.END`, `DMA.TX.END`)

These events indicate that EasyDMA has finished accessing the RAM.

## For DMIC/PDM Use

The PDM (Pulse Density Modulation) peripheral, which drives DMIC microphones, uses EasyDMA to:

1. **Capture audio samples directly into RAM buffers**
2. **Support double-buffering** for continuous capture
3. **Minimize CPU intervention** during audio capture
4. **Enable efficient producer-consumer patterns** with ring buffers

### Best Practices for DMIC + EasyDMA

1. **Use mem_slab** for DMA buffer pool management
2. **Implement ring buffer** for producer-consumer pattern between DMIC and LC3 encoder
3. **Match frame sizes** to LC3 requirements (10ms frames @ 16kHz = 160 samples)
4. **Monitor buffer levels** to prevent overflows
5. **Free DMA buffers** back to mem_slab after copying to ring buffer

## References

- [Nordic EasyDMA Documentation](https://docs.nordicsemi.com/category/easydma)
- nRF54L15 Product Specification
- See each peripheral's datasheet for specific EasyDMA implementation details

---

## Implementation Analysis vs Nordic Documentation

### ✅ What We Implemented CORRECTLY:

1. **Memory Slab (DMA Buffer Pool)** ✅
   ```c
   K_MEM_SLAB_DEFINE_STATIC(dma_mem_slab, BLOCK_SIZE_BYTES, BLOCK_COUNT, 4);
   ```
   - **Matches Nordic**: The PDM driver's `.mem_slab` is the EasyDMA buffer pool
   - **Purpose**: PDM peripheral uses EasyDMA to write directly into these buffers
   - **Correct**: 4 blocks for continuous operation (double-buffering support)

2. **Buffer Configuration** ✅
   ```c
   struct pcm_stream_cfg stream = {
       .pcm_width = SAMPLE_BIT_WIDTH,
       .mem_slab = &dma_mem_slab,  // This is the PTR equivalent
   };
   cfg.streams[0].block_size = BLOCK_SIZE_BYTES;  // This is MAXCNT equivalent
   ```
   - **Matches Nordic**: `.mem_slab` provides buffers (PTR), `.block_size` sets transfer size (MAXCNT)
   - **PDM driver handles**: The low-level `.PTR` and `.MAXCNT` register configuration

3. **DMA Read Operation** ✅
   ```c
   dmic_read(g_dmic_dev, 0, &buffer, &size, SYS_FOREVER_MS);
   ```
   - **Returns**: Pointer to DMA buffer that peripheral filled via EasyDMA
   - **Zero-copy**: Buffer is directly from mem_slab, no CPU copying during capture

4. **Buffer Free Back to Pool** ✅
   ```c
   k_mem_slab_free(&dma_mem_slab, buffer);
   ```
   - **Correct**: After copying to ring buffer, free DMA buffer back to pool
   - **Matches Nordic**: Allows hardware to reuse buffer for next capture

5. **Producer-Consumer Pattern** ✅
   ```c
   RING_BUF_DECLARE(audio_ring_buf, RING_BUF_SIZE);
   ```
   - **Correct**: DMA buffers → Ring buffer → LC3 encoder
   - **Decouples**: DMA capture rate from LC3 encoding rate
   - **Overflow protection**: Monitors ring buffer space before writing

### ❌ What We're NOT Using (But PDM Driver Handles Internally):

1. **Array List Mode** - NOT APPLICABLE FOR PDM
   - PDM driver on nRF54L doesn't expose Array List mode to application
   - Driver internally manages buffer rotation using mem_slab

2. **Manual PTR/MAXCNT Configuration** - NOT NEEDED
   - PDM driver abstracts this away
   - We configure via high-level `pcm_stream_cfg` structure

3. **AMOUNT Register Reading** - HIDDEN BY DRIVER
   - Driver returns `size` parameter in `dmic_read()`
   - Application doesn't need to read registers directly

### 🎯 How Our Code Maps to Nordic EasyDMA Concept:

**Nordic Documentation Example:**
```c
MYPERIPHERAL->READER.MAXCNT = READERBUFFER_SIZE;
MYPERIPHERAL->READER.PTR = &readerBuffer;
```

**Our PDM/DMIC Implementation:**
```c
// High-level equivalent (PDM driver does the low-level work):
cfg.streams[0].block_size = BLOCK_SIZE_BYTES;  // → PDM.SAMPLE.MAXCNT
stream.mem_slab = &dma_mem_slab;               // → PDM.SAMPLE.PTR (from slab)
```

### 📊 Memory Layout (Matches Nordic Documentation):

**From dmic_read():**
```
0x20000000: readerBuffer[0] ──┐
0x20000140: readerBuffer[1]   ├─ dma_mem_slab (4 blocks of 320 bytes)
0x20000280: readerBuffer[2]   │  ← PDM peripheral writes via EasyDMA
0x200003C0: readerBuffer[3] ──┘

                ↓ (copy after read)

0x20001000: audio_ring_buf ──── 2560 bytes ring buffer
                                 ← Application reads for LC3 encoder
```

### ✅ CONCLUSION: Implementation is CORRECT

Our code properly implements EasyDMA concepts for PDM/DMIC:

1. ✅ **DMA buffers allocated in RAM** (mem_slab in RAM00/RAM01)
2. ✅ **PDM peripheral writes directly via EasyDMA** (no CPU involvement during capture)
3. ✅ **Zero-copy from peripheral to DMA buffer** (pointer returned by dmic_read)
4. ✅ **Copy to ring buffer happens in application thread** (not during DMA)
5. ✅ **Buffers recycled back to pool** (k_mem_slab_free)
6. ✅ **Producer-consumer pattern** (decouples capture from processing)

**The Zephyr PDM driver handles all the low-level EasyDMA register configuration (.PTR, .MAXCNT, .AMOUNT) internally, and we correctly use the high-level API that leverages EasyDMA underneath.**
