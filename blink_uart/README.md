# UART Checkpoint Sample - System Off with GPIO Sense Wake

This sample demonstrates a battery-powered "checkpoint" device that:

1. **Sleeps in system-off mode** - Lowest power consumption
2. **Monitors P2.07 with GPIO sense** - Wakes on high signal
3. **Sends a secret message** - "hi my name is slim shady" at 115200 baud via UART21
4. **Returns to sleep** - Conserves battery power

## Hardware Configuration

- **UART21 TX**: P2.08 (transmit only)
- **Wake Pin**: P2.07 (GPIO sense input, always)
- **LED**: Built-in LED (led0 alias)
- **Baud Rate**: 115200, 8N1, no flow control
- **Note**: UART RX is disconnected; P2.07 is dedicated to GPIO wake

## Operation

1. On startup:
   - P2.07 is configured as GPIO sense input (wake on high)
   - LED blinks 3 times rapidly (startup indicator)

2. On boot or wake from system-off:
   - LED turns on solid
   - UART21 TX is enabled on P2.08
   - Message is transmitted **3 times** (TX-only mode)
   - Delays between transmissions ensure completion in RTOS
   - UART21 is disabled
   - LED turns off
   - Device enters system-off mode

3. To wake the device:
   - Apply a high signal (3.3V) to P2.07
   - Device will wake and repeat the transmission sequence

## Building and Flashing

```bash
cd blink_uart
west build -b xiao_nrf54l15/nrf54l15/cpuapp -p
west flash
```

## Testing

Connect a USB-to-serial adapter to:
- **RX** → P2.08 (UART TX from nRF54L15)
- **GND** → GND
- Configure terminal: 115200 baud, 8N1

To trigger wake:
- Connect P2.07 to 3.3V momentarily

## Power Consumption

- **Active (transmitting)**: ~100ms per wake cycle
- **Sleep (system-off)**: < 1 µA (GPIO sense enabled)