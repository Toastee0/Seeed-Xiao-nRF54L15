# Channel Sounding Distance LED Indicator

This sample combines Bluetooth Channel Sounding distance measurement with visual LED feedback on the Xiao nRF54L15.

## Features

- **Channel Sounding Distance Measurement**: Uses calibrated IFFT, Phase Slope, and RTT methods with weighted fusion
- **Visual Distance Indicator**: RGB LED shows distance to reflector target
- **Connection Status**: Red pulsing when disconnected, color-coded distance when connected

## LED Behavior

### Disconnected State
- **Red LED**: Pulses from 0 to 128 brightness over 1 second period
- Green and Blue are off

### Connected State
- **Red LED**: Always off
- **Green LED**: Brightest at close range (0-5m)
  - Maximum brightness at 0m
  - Fades as distance increases to 5m
- **Blue LED**: Brightest at far range (5-10m)
  - Increases from 5m to 10m
  - Maximum brightness beyond 10m

## Distance Mapping

| Distance Range | Green Brightness | Blue Brightness | Interpretation |
|----------------|------------------|-----------------|----------------|
| 0-1m          | 255 (Max)        | 25              | Very close     |
| 1-3m          | 200-150          | 50-75           | Close          |
| 3-5m          | 100-0            | 100-128         | Medium         |
| 5-7m          | 50-0             | 150-200         | Far            |
| 7-10m         | 0                | 200-255         | Very far       |
| >10m          | 0                | 255 (Max)       | Maximum range  |

## Hardware Requirements

- **Xiao nRF54L15** board
- **WS2812 RGB LED** connected to P2.08 (D6/TX)
- **Reflector target** running Channel Sounding reflector firmware

## Calibration

The sample uses calibrated distance formulas derived from controlled measurements:
- Phase Slope: slope=1.786, offset=1.307m, weight=50%
- RTT: slope=1.800, offset=3.200m, weight=25%
- IFFT: slope=1.600, offset=1.400m, weight=25%

## Building and Flashing

```bash
cd bluetooth/cs_distance_led
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Usage

1. Flash reflector firmware to second Xiao nRF54L15
2. Flash this initiator firmware to first Xiao nRF54L15
3. Power on both devices
4. Initiator will scan and connect to reflector
5. Watch LED color change based on distance:
   - **Red pulsing** = Not connected
   - **Green** = Close (0-5m)
   - **Blue** = Far (5-10m+)

## Technical Details

- Uses manual RF switch GPIO control (P2.3=HIGH, P2.5=LOW for onboard antenna)
- Full 256KB SRAM allocation to cpuapp
- SoftDevice Controller only (no split configuration)
- Distance averaging over 10 measurements for stability
- 50ms LED update rate for smooth visual feedback
