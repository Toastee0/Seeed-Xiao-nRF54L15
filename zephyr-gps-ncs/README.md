# GPS Module (L76X) Sample

This sample demonstrates how to interface with an L76X GPS module using the Seeed XIAO nRF54L15 development board. The sample reads NMEA sentences from the GPS module and performs coordinate transformations between different coordinate systems.

## Features
- NMEA sentence parsing (GNRMC)
- WGS-84 coordinate reading
- Coordinate conversion to GCJ-02 (Google/Amap coordinates)
- Coordinate conversion to BD-09 (Baidu coordinates)
- UART interrupt-driven reception
- Real-time GPS data display

## Hardware Requirements
- Seeed XIAO nRF54L15 development board
- L76X GPS module (or compatible NMEA GPS module)
- Jumper wires for connections

## Wiring
Connect the GPS module to the XIAO Serial port:
- GPS VCC → 3.3V
- GPS GND → GND  
- GPS TX → XIAO Serial RX
- GPS RX → XIAO Serial TX

## Building and Running

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The sample will output GPS information including:
- Current time (GMT+8)
- Latitude and longitude in WGS-84 format
- Converted coordinates for Baidu maps
- Converted coordinates for Google maps
- GPS positioning status

Example output:
```
[00:00:01.234,567] <inf> gps_app: Starting L76X GPS Module Example
[00:00:01.234,789] <inf> gjs_app: UART device initialized.
[00:00:01.235,012] <inf> gps_app: GPS module initialized. Waiting for data...
[00:00:05.678,901] <inf> gps_app: Raw GNRMC: $GNRMC,123456.00,A,3112.34567,N,12123.45678,E,0.00,0.00,010101,,,*XX
[00:00:05.679,123] <inf> gps_app: --- GPS Data ---
[00:00:05.679,345] <inf> gps_app: Time (GMT+8): 20:34:56
[00:00:05.679,567] <inf> gps_app: Latitude (WGS-84): 31.205761 N
[00:00:05.679,789] <inf> gps_app: Longitude (WGS-84): 121.390946 E
[00:00:05.680,012] <inf> gps_app: Baidu Latitude: 31.211234
[00:00:05.680,234] <inf> gps_app: Baidu Longitude: 121.396789
[00:00:05.680,456] <inf> gps_app: Google Latitude: 31.207890
[00:00:05.680,678] <inf> gps_app: Google Longitude: 121.394567
[00:00:05.680,901] <inf> gps_app: GPS positioning successful.
```

## Notes
- The GPS module requires a clear view of the sky for proper operation
- Initial GPS lock may take several minutes (cold start)
- Coordinate transformations are specifically for Chinese coordinate systems
- The sample uses interrupt-driven UART for efficient data reception
- UART is configured for 9600 baud rate to match most GPS modules