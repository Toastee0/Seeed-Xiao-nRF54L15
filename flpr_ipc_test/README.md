# FLPR IPC Test Application

This application demonstrates Inter-Processor Communication (IPC) between the CPU application core and the Fast Lightweight Processor (FLPR) core on the nRF54L15.

## Features

- **Bidirectional IPC Communication**: CPU app sends data to FLPR, FLPR processes and sends back modified data
- **Dynamic Data**: Counter increments with each transmission to prove cores are working in concert
- **Data Transformation**: FLPR core doubles the counter value and adds offset to timestamp
- **Visual Feedback**: LED toggles on successful round-trip communication
- **Comprehensive Logging**: Both cores log their activities

## Architecture

### CPU Application Core (`src/main.c`)
- Sends incrementing counter data packets to FLPR every 2 seconds
- Each packet contains: counter, timestamp, message string
- Receives modified data back from FLPR
- Logs full round-trip information
- Toggles LED on successful communication

### FLPR Core (`remote/src/main.c`)
- Receives data packets from CPU app via IPC
- Processes data: doubles counter, adds 1000 to timestamp
- Sends modified packet back to CPU app
- Logs all operations

## Data Structure

```c
struct ipc_data_packet {
    uint32_t counter;        // Incremented by CPU app
    uint32_t timestamp;      // Current uptime
    char message[32];        // Descriptive message
};
```

## Building

### For Seeed XIAO nRF54L15:
```bash
west build -p -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

### For nRF54L15 DK:
```bash
west build -p -b nrf54l15dk/nrf54l15/cpuapp
west flash
```

## Expected Output

```
CPU APP: FLPR IPC Test Application
==========================================
CPU APP: LED initialized
CPU APP: Waiting for endpoint to bind...
FLPR: IPC endpoint bound
CPU APP: IPC endpoint bound
CPU APP: IPC ready! Starting data exchange...
==========================================
CPU APP: Sending counter=0, timestamp=1234, msg='APP_MSG_0'
FLPR: Received counter=0, timestamp=1234, msg='APP_MSG_0'
FLPR: Sent back counter=0, timestamp=2234, msg='FLPR_ACK_0'
CPU APP: Response from FLPR - counter=0, timestamp=2234, msg='FLPR_ACK_0'
CPU APP: ✓ Round-trip successful!
CPU APP:   Sent:     counter=0
CPU APP:   Received: counter=0 (doubled by FLPR)
CPU APP:   Timestamp delta: 1000 ms
------------------------------------------
```

## Testing

1. Flash the application to your device
2. Connect to UART console (115200 baud)
3. Observe:
   - Counter incrementing with each transmission
   - FLPR doubling the counter value
   - Timestamp deltas showing processing time
   - LED blinking on successful communication

## Notes

- Both cores must be flashed for the application to work
- The sysbuild system automatically builds and flashes both images
- IPC uses shared memory region at 0x2003e000
- Uses OpenAMP with static vrings for reliable communication
