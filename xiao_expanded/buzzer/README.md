# Expansion Base Buzzer Sample

This sample demonstrates PWM-based buzzer control on the Seeed XIAO nRF54L15 expansion base. Press the button to activate the buzzer.

## Features

- **Button-triggered buzzer** - Press SW0 (D1) to sound the buzzer
- **PWM-driven buzzer** - Uses PWM on D3 for precise frequency control
- **1kHz buzzer frequency** - Clear audible tone
- **50% duty cycle** - Balanced sound output
- **Button polling** - Simple button state monitoring

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- XIAO Expansion Base (or buzzer connected to D3)
- Buzzer (passive or active)

## Wiring

| Component | XIAO Pin | GPIO Pin | Function    |
|-----------|----------|----------|-------------|
| Button    | D1       | P1.05    | Input       |
| Buzzer    | D3       | P1.07    | PWM Output  |

**Note**: If using the expansion base, the buzzer is already connected to D3.

## Building and Running

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

## Expected Behavior

1. Power on the board
2. Serial output shows: "Press SW0 to activate the buzzer."
3. Press and hold the button on D1
4. Buzzer sounds at 1kHz
5. Release button
6. Buzzer stops

## Console Output

```
Buzzer example started
Press SW0 to activate the buzzer.
Button pressed, starting buzzer...
```

## Configuration

- **Buzzer Frequency**: 1000 Hz (adjustable via `BUZZER_FREQUENCY_HZ`)
- **PWM Duty Cycle**: 50% (adjustable via `BUZZER_DUTY_CYCLE_PERCENT`)
- **Button Pin**: D1 (P1.05) with pull-up resistor
- **Buzzer Pin**: D3 (P1.07) - PWM Channel 0
- **Button Poll Rate**: 20ms

## Technical Details

### PWM Configuration
- Uses Nordic PWM20 peripheral
- PWM Channel 0 output on P1.07 (D3)
- Period calculated based on desired frequency
- Pulse width set to duty cycle percentage

### Button Configuration
- GPIO input with internal pull-up
- Active-low configuration
- Polled every 20ms to reduce CPU usage

## Customization

You can adjust the buzzer frequency by changing the constant:

```c
#define BUZZER_FREQUENCY_HZ 1000  // Change to desired frequency (Hz)
```

Common buzzer frequencies:
- **440 Hz** - Musical note A4
- **1000 Hz** - Clear audible tone (default)
- **2000 Hz** - Higher pitched tone
- **4000 Hz** - Alarm-like tone

## Troubleshooting

- **No sound**: Check buzzer connection to D3 and polarity (if using passive buzzer)
- **Weak sound**: Adjust duty cycle percentage or use external amplification
- **Button not responding**: Verify button connection to D1 and pull-up configuration
- **Continuous sound**: Check for stuck button or connection issue
