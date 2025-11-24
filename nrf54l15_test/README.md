# nRF54L15 Test Application

A test application for the Seeed Studio Xiao nRF54L15 that demonstrates:
- UART communication with interrupt handling
- Hardware ID retrieval and reporting
- Version information display
- JSON message formatting
- Power management for UART domains

## Features

- **Hardware ID Query**: Send "getID" to receive hardware ID in JSON format
- **Version Query**: Send "getVERSION" to receive version information in JSON format
- **UART21 Interface**: Uses P2.08 (TX) and P2.07 (RX) pins
- **Power Management**: Implements constant latency mode for reliable UART operation

## Building

Use the nRF Connect SDK build commands:

```bash
west build --build-dir build --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
west flash --build-dir build
```

## Usage

Connect to the UART21 interface (115200 baud) and send:
- `getID` - Returns hardware ID
- `getVERSION` - Returns version information

Both commands return JSON formatted responses.

## Notes

The application uses `nrfx_power_constlat_mode_request()` to ensure reliable UART operation
across different power domains on the nRF54L15.