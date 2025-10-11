# Blink UART Sample - LED Blink with UART Output and System Off

This sample demonstrates a simple LED blink application with UART output that runs for a fixed number of iterations before entering low-power system-off mode.

## Overview

The sample performs the following operations:

1. **Initial delay** - Waits 1 second after startup
2. **LED blinks 6 times** - Toggles the LED with 1 second intervals
3. **UART messages** - Prints alternating "Slim Shady" messages during each blink
4. **System off** - Enters low-power system-off mode after completing all blinks

## How It Works

### Main Loop Sequence

1. **Initialization**:
   - 1 second startup delay
   - Configures the built-in LED (led0) as an output
   - LED starts in ON state (active)

2. **Blink Loop** (6 iterations):
   - Toggles the LED state
   - Prints alternating messages to UART:
     - "Yes the real Slim Shady!" (LED OFF)
     - "I'm Slim Shady" (LED ON)
   - Waits 1000ms between each toggle
   - Increments counter

3. **Shutdown**:
   - Sets LED to ON state
   - Prints "Entering system off mode..." to UART
   - Waits 100ms for UART transmission to complete
   - Enters system-off mode (lowest power state)

### Power Management

The sample uses Zephyr's power management subsystem:
- **CONFIG_PM=y** - Enables power management
- **CONFIG_POWEROFF=y** - Enables the `sys_poweroff()` function

Once in system-off mode, the device consumes minimal power. To restart the application, you must reset the device (power cycle or press the reset button).

## Hardware Configuration

- **LED**: Built-in LED (led0 alias) - typically the board's user LED
- **UART**: Console UART for debug output (typically USB-CDC or hardware UART)
- **Baud Rate**: 115200, 8N1 (default console settings)

## Building and Flashing

```bash
cd blink_uart
west build -b xiao_nrf54l15/nrf54l15/cpuapp -p
west flash
```

Or use the nRF Connect extension in VS Code.

## Testing

1. **Connect to UART console**:
   - Use a serial terminal (PuTTY, screen, minicom, etc.)
   - Connect to the device's console UART
   - Set baud rate to 115200, 8N1

2. **Observe the output**:
   - After reset, wait 1 second
   - You'll see 6 alternating messages printed
   - LED will blink in sync with messages
   - Final message: "Entering system off mode..."
   - Device enters low-power mode

3. **Expected Output**:
   ```
   Yes the real Slim Shady!
   
   I'm Slim Shady
   
   Yes the real Slim Shady!
   
   I'm Slim Shady
   
   Yes the real Slim Shady!
   
   I'm Slim Shady
   
   Entering system off mode...
   ```

4. **To restart**: Press the reset button or power cycle the board

## Configuration Options

You can modify the following defines in `main.c`:
- **SLEEP_TIME_MS** (1000) - Delay between LED toggles in milliseconds
- **LOOP_COUNT** (6) - Number of times to blink the LED

## Power Consumption

- **Active (running)**: Standard operating current (~6 seconds total runtime)
- **System-off mode**: < 1 µA (minimal power consumption)
- **Note**: To wake from system-off, a hardware reset is required (no GPIO wake configured in this sample)