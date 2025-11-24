# Basic BLE Sample for Xiao nRF54L15

This is a basic Bluetooth Low Energy (BLE) beacon sample for the Seeed Studio Xiao nRF54L15 board.

## Features

- BLE non-connectable advertising
- Custom device name broadcasting
- Scan response with URL data
- LED blinking to indicate operation

## Building

Build for the Xiao nRF54L15:
```
west build -b xiao_nrf54l15/nrf54l15/cpuapp
```

## Flashing

```
west flash
```

## Testing

Use a BLE scanner app (e.g., nRF Connect for Mobile) to see the advertising device named "Xiao_nRF54L15_Beacon".
