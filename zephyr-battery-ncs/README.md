# Battery Voltage Monitor Sample

This sample demonstrates battery voltage monitoring using ADC with regulator control on the Seeed XIAO nRF54L15.

## Features

- ADC-based battery voltage measurement
- Regulator control for power management
- Voltage scaling and conversion to millivolts
- Single-shot measurement with proper cleanup

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- Battery voltage divider circuit connected to ADC channel 7
- Built-in battery voltage regulator (vbat_pwr)

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The sample will read the battery voltage once and display the result:

```
bat vol = 3300 mV
```

## Configuration

- ADC is enabled for voltage measurement
- ADC async mode is enabled for better performance
- GPIO is enabled for regulator control
- Uses ADC channel 7 for battery voltage measurement
- Applies 2x scaling factor to account for voltage divider

## Technical Details

- Enables battery voltage regulator before measurement
- Waits 100ms for regulator stabilization
- Reads raw ADC value and converts to millivolts
- Applies 2x multiplication for actual battery voltage
- Disables regulator after measurement to save power