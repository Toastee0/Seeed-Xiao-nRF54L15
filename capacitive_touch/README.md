# Capacitive Touch Sample

This sample demonstrates capacitive touch sensing on the Seeed Studio XIAO nRF54L15 using the built-in COMP (Comparator) peripheral. The application can detect touch events on pins D0, D1, D2, and D3 simultaneously.

## Overview

The nRF54L15 COMP peripheral is specifically designed for capacitive touch sensing applications. This sample configures the comparator in single-ended mode to detect voltage changes when a finger approaches or touches a sensor pad.

### Key Features

- **4-channel capacitive touch sensing** on D0-D3 pins
- **Hardware-accelerated detection** using COMP peripheral
- **ADC fallback method** for enhanced reliability
- **Automatic baseline calibration** at startup
- **Debounced touch detection** to prevent false triggers
- **Visual feedback** via built-in LED
- **Real-time logging** of touch events and sensor readings

## Hardware Requirements

- Seeed Studio XIAO nRF54L15 development board
- Touch sensor pads (copper plates, PCB traces, or wires)
- Breadboard or PCB for sensor pad mounting
- USB-C cable for power and programming

## Pin Mapping

The sample uses the following pins for capacitive touch sensing:

| Header Pin | GPIO Pin | ADC Channel | Touch Channel | Description |
|------------|----------|-------------|---------------|-------------|
| D0         | P1.04    | AIN0        | Touch 0       | Capacitive touch sensor 0 |
| D1         | P1.05    | AIN1        | Touch 1       | Capacitive touch sensor 1 |
| D2         | P1.06    | AIN2        | Touch 2       | Capacitive touch sensor 2 |
| D3         | P1.07    | AIN3        | Touch 3       | Capacitive touch sensor 3 |
| Built-in LED | P2.0   | -           | Visual        | Touch indication LED |

## Touch Sensor Pad Construction

### Simple Wire Sensors
For testing, you can create basic touch sensors using regular wires:

1. Cut 4 pieces of wire, each about 10cm long
2. Strip 2-3cm of insulation from one end of each wire
3. Connect the stripped ends to pins D0, D1, D2, D3 respectively
4. Leave the other ends as exposed copper for touch sensing

### PCB Touch Pads
For better sensitivity, create proper PCB touch pads:

1. Design circular or rectangular copper areas (5-20mm diameter/width)
2. Connect via traces to the XIAO pins D0-D3
3. Optionally add a solder mask opening over the copper
4. Keep the copper exposed for direct finger contact

### Optimal Sensor Design Guidelines
- **Pad size**: 8-15mm diameter for finger touch
- **Isolation**: Keep sensor pads separated by at least 5mm
- **Ground plane**: Add a ground plane on the opposite PCB layer
- **Sensitivity**: Larger pads = higher sensitivity but lower selectivity

## Wiring Diagram

```
    XIAO nRF54L15
    ┌─────────────────┐
    │     ┌─────┐     │
    │  D0 │ USB │ 3V3 │ ◄── Touch Pad 0
    │  D1 │     │ GND │ ◄── Touch Pad 1  
    │  D2 │ LED │ D10 │ ◄── Touch Pad 2
    │  D3 │     │ D9  │ ◄── Touch Pad 3
    │  D4 └─────┘ D8  │
    │  D5       D7    │
    │  D6             │
    └─────────────────┘
```

## Building and Running

### Prerequisites
Ensure you have the nRF Connect SDK properly configured as described in the main README.

### Build Commands

**IMPORTANT**: First ensure your nRF Connect SDK environment is properly configured as described in the [nRF54L15 Development Guide](../docs/nrf54l15-guide-compressed.md).

```powershell
# Method 1: From capacitive_touch directory
cd m:\nRF54L15\capacitive_touch
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash

# Method 2: From repository root directory  
west build --build-dir capacitive_touch/build capacitive_touch -b xiao_nrf54l15/nrf54l15/cpuapp
west flash --build-dir capacitive_touch/build

# Method 3: Using full sysbuild approach
west build --build-dir capacitive_touch/build capacitive_touch --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --sysbuild
west flash --build-dir capacitive_touch/build
```

## Usage Instructions

### 1. Initial Setup
1. Connect your touch sensor pads to pins D0-D3 as described above
2. Power on the XIAO nRF54L15 board
3. Open a serial terminal at 115200 baud to view the output

### 2. Calibration Phase
When the application starts:
1. The system will display: "Please ensure no fingers are touching sensor pads..."
2. **IMPORTANT**: Do not touch any sensor pads for 3 seconds during this phase
3. The system will automatically calibrate baseline values for each channel
4. You'll see log messages showing the baseline values for each channel

### 3. Normal Operation
After calibration:
- Touch any of the D0-D3 sensor pads
- The built-in LED will illuminate when any touch is detected
- Touch events are logged to the serial console
- Multiple simultaneous touches are supported

## Expected Output

### Serial Console Log
```
[00:00:00.123,456] <inf> capacitive_touch: Starting Capacitive Touch Sample
[00:00:00.123,789] <inf> capacitive_touch: nRF54L15 COMP-based touch sensing on D0-D3
[00:00:00.124,012] <inf> capacitive_touch: LED initialized on P2.0
[00:00:00.124,345] <inf> capacitive_touch: Touch sensor pins initialized (D0-D3)
[00:00:00.124,678] <inf> capacitive_touch: ADC channels initialized
[00:00:00.124,901] <inf> capacitive_touch: COMP peripheral initialized for capacitive touch
[00:00:00.125,234] <inf> capacitive_touch: Please ensure no fingers are touching sensor pads...
[00:00:03.125,567] <inf> capacitive_touch: Calibrating touch sensor baselines...
[00:00:03.225,890] <inf> capacitive_touch: Channel 0 baseline: 1024
[00:00:03.326,123] <inf> capacitive_touch: Channel 1 baseline: 1018
[00:00:03.426,456] <inf> capacitive_touch: Channel 2 baseline: 1031
[00:00:03.526,789] <inf> capacitive_touch: Channel 3 baseline: 1027
[00:00:03.527,012] <inf> capacitive_touch: Touch sensor calibration complete
[00:00:03.527,345] <inf> capacitive_touch: Touch detection active - touch D0, D1, D2, or D3 pads
[00:00:03.527,678] <inf> capacitive_touch: LED will light up when any touch is detected

# When touching sensor pads:
[00:00:05.123,456] <inf> capacitive_touch: Touch detected on channel 0
[00:00:06.234,567] <inf> capacitive_touch: Touch released on channel 0
[00:00:07.345,678] <inf> capacitive_touch: Touch detected on channel 2
[00:00:07.456,789] <inf> capacitive_touch: Touch detected on channel 1  # Multi-touch

# Periodic status updates:
[00:00:09.567,890] <inf> capacitive_touch: Touch Status - D0:OFF D1:ON  D2:ON  D3:OFF
[00:00:09.568,123] <inf> capacitive_touch: ADC Readings - D0:1024 D1:1156 D2:1203 D3:1027
```

## How It Works

### COMP Peripheral Operation
The COMP (Comparator) peripheral compares the voltage on an analog input pin (VIN+) against a reference voltage (VIN-). For capacitive touch sensing:

1. **VIN+**: Connected to touch sensor pad (AIN0-AIN3)
2. **VIN-**: Internal VDD reference (typically VDD/2)
3. **Detection**: Voltage changes when finger approaches due to capacitive coupling
4. **Hysteresis**: 50mV hysteresis prevents noise-induced false triggers

### Touch Detection Algorithm
The application uses a dual-method approach:

1. **Primary (COMP)**: Hardware comparator for fast, low-power detection
2. **Fallback (ADC)**: Software-based threshold comparison using ADC readings
3. **Baseline**: Initial calibration establishes untouched reference values
4. **Debounce**: 100ms debounce period prevents bounce artifacts

### Multi-Channel Scanning
- Each channel is sampled sequentially every 50ms
- COMP PSEL register is reconfigured for each channel
- State tracking maintains touch status for all channels simultaneously
- LED provides aggregate visual feedback (on if any channel touched)

## Configuration Options

### Sensitivity Adjustment
Modify these constants in `main.c` to adjust sensitivity:

```c
#define TOUCH_THRESHOLD_MV 100    // Lower = more sensitive
#define SAMPLE_INTERVAL_MS 50     // Lower = more responsive  
#define TOUCH_DEBOUNCE_MS 100     // Higher = less bouncy
```

### COMP Parameters
Advanced users can modify COMP configuration:

```c
#define COMP_HYST_50MV 2          // Hysteresis (0=NoHyst, 1=30mV, 2=50mV)
#define COMP_REF_VDD_DIV2 4       // Reference selection
#define COMP_SPEED_NORMAL 0       // Speed vs power trade-off
```

## Troubleshooting

### No Touch Detection
- **Check wiring**: Ensure sensor pads are connected to correct pins
- **Recalibrate**: Restart application without touching pads during calibration
- **Sensitivity**: Try reducing `TOUCH_THRESHOLD_MV` value
- **Sensor size**: Use larger sensor pads (10-15mm diameter)

### False Triggers
- **Environmental**: Move away from electromagnetic interference sources
- **Increase threshold**: Raise `TOUCH_THRESHOLD_MV` value  
- **Debounce**: Increase `TOUCH_DEBOUNCE_MS` value
- **Shielding**: Add ground plane on PCB opposite side

### Inconsistent Response
- **Power supply**: Ensure stable 3.3V supply
- **Grounding**: Verify good system ground connection
- **Calibration**: Allow proper calibration time at startup
- **Sampling rate**: Adjust `SAMPLE_INTERVAL_MS` if needed

### Build Errors
- **SDK version**: Ensure nRF Connect SDK v2.7.0+ is installed
- **Device tree**: Verify overlay file is correctly named and placed
- **COMP support**: Confirm COMP peripheral is available on your nRF54L15 variant

## Technical Notes

### Power Consumption
- **COMP peripheral**: ~10µA in normal mode
- **Sampling overhead**: Additional ~50µA during active scanning
- **Sleep optimization**: Could be reduced by implementing proper sleep modes

### Response Time
- **COMP settling**: ~20µs per channel sample
- **Total scan time**: ~80µs for all 4 channels
- **Update rate**: 50ms interval (20Hz effective sample rate)

### Hardware Limitations
- **Maximum channels**: 8 total AIN pins, 4 used in this sample
- **Reference accuracy**: ±1% typical for internal references
- **Temperature drift**: ~100ppm/°C for VDD reference

## Extensions and Improvements

### Possible Enhancements
1. **Proximity sensing**: Detect finger approach before touch
2. **Multi-level touch**: Distinguish light vs firm touch pressure
3. **Gesture recognition**: Implement swipe/pinch patterns
4. **Sleep mode**: Reduce power consumption between scans
5. **Bluetooth**: Transmit touch events wirelessly
6. **Matrix scanning**: Support larger sensor arrays

### Advanced Applications  
- **Piano keyboard**: Musical instrument interface
- **Control panel**: Industrial control applications
- **Gaming controller**: Custom game input device
- **IoT sensor**: Remote touch monitoring over wireless

## References

- [nRF54L15 Product Specification](https://infocenter.nordicsemi.com/pdf/nRF54L15_PS_v0.5.pdf)
- [nRF Connect SDK Documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/)
- [Capacitive Sensing Theory](https://www.cypress.com/file/178781/download)
- [XIAO nRF54L15 Schematic](https://files.seeedstudio.com/wiki/XIAO-nRF54L15/XIAO_nRF54L15_v1.0_SCH_240429.pdf)