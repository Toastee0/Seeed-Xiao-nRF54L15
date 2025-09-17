# ADC to PWM Control Sample

This sample demonstrates how to read analog values from a potentiometer using ADC and control LED brightness with PWM on the Seeed XIAO nRF54L15.

## Features

- ADC reading from potentiometer
- Real-time PWM control of LED brightness
- Voltage conversion and logging
- Configurable ADC channels

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- Potentiometer connected to ADC channel 1
- LED connected to PWM channel

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). To build:

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Output

The sample will continuously read the potentiometer value and adjust the LED brightness accordingly. Console output will show:

```
Starting Zephyr Potentiometer to PWM example...
ADC device ready for potentiometer
PWM Period for LED set to 1000000 ns (1000.0 Hz)
Sensor Raw Value = 512 (1650 mV)    Output Duty (ns) = 500000
```

## Configuration

The ADC and PWM configurations can be modified in the device tree overlay file for different hardware setups.