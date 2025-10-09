# Grove Relay Control Sample

This sample demonstrates GPIO input and output operations using dual button control for a relay on the Seeed XIAO nRF54L15 with expansion base.

## Features

- **Toggle Control**: Built-in XIAO button (sw0) toggles relay on/off state
- **Momentary Control**: External button on D1 (sw1) provides momentary relay activation
- GPIO output for relay control (relay0 on expansion base A0/D0 port)
- Real-time button state monitoring with debug logging
- Console logging for status updates and button actions
- Error handling for GPIO operations
- Internal pull-up resistor configuration for external button

## Hardware Requirements

- Seeed XIAO nRF54L15 development board
- Seeed XIAO Expansion Base
- Grove relay module connected to expansion base A0/D0 port
- External button/switch connected to D1 pin (optional - for momentary control)

## Building and Running

This sample is built using the Nordic Connect SDK (NCS). **No user configuration is required** - all necessary configuration files are included in the `boards/` subfolder.

### Using nRF Connect for VS Code (Recommended)
1. Open the sample folder in VS Code
2. The sample will automatically appear in nRF Connect for VS Code
3. Add build configuration for `xiao_nrf54l15/nrf54l15/cpuapp` board
4. Build and flash using the nRF Connect extension

### Using Command Line
```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
west flash
```

### Configuration Files Included
- `boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay` - Device tree configuration for GPIO pins and aliases
- `boards/xiao_nrf54l15_nrf54l15_cpuapp.conf` - sample specific configuration options, overrides prj.conf
- `prj.conf` - Project configuration enabling GPIO and logging (empty but required by NCS)
- `sample.yaml` - Sample metadata for nRF Connect for VS Code integration (allows selecting these as samples using ncs extension)

## Button Behavior

### Built-in XIAO Button (sw0) - Toggle Mode
- **First press**: Toggles relay ON, logs "relay on (built-in XIAO button toggle)"
- **Second press**: Toggles relay OFF, logs "relay off (built-in XIAO button toggle)"
- **State persists**: Relay maintains its state until button is pressed again

### External D1 Button (sw1) - Momentary Mode  
- **While pressed**: Forces relay ON, logs "relay on (D1 button momentary)"
- **When released**: Relay returns to the current toggle state, logs "relay restored to toggle state"
- **Overrides toggle**: Momentary control takes priority over toggle state while active

## Expected Output

Console output will show:

```
Starting Zephyr button and relay example...
Press the built-in XIAO button (sw0) to toggle the relay...
Hold the external button on D1 (sw1) for momentary relay control...
Button states - sw0 (built-in): 1, sw1 (D1): 0
[Button press examples]
relay on (built-in XIAO button toggle)
relay on (D1 button momentary)
relay restored to toggle state
```

## Configuration

- GPIO is enabled for dual button input and relay output
- Logging is enabled at info level for status monitoring and debugging  
- Uses device tree aliases:
  - `sw0` for built-in XIAO button (references existing `usr_btn`)
  - `sw1` for external D1 button (custom definition)
  - `relay0` for Grove relay module on expansion base
- Internal pull-up resistor enabled for D1 button
- Debug logging shows button states every 5 seconds

## Device Tree Configuration

The sample uses the following device tree configuration:

```dts
/ {
    aliases {
        sw0 = &usr_btn;          // Built-in XIAO button
        sw1 = &xiao_btn;         // External D1 button  
        relay0 = &relay_0;       // Grove relay
    };

    buttons {
        compatible = "gpio-keys";
        xiao_btn: xiao_button {
            gpios = <&xiao_d 1 GPIO_ACTIVE_HIGH>;
            label = "XIAO Button D1";
        };
    };

    relays {
        compatible = "gpio-leds";
        relay_0: relay_0 {
            gpios = <&xiao_d 0 GPIO_ACTIVE_LOW>;  // Expansion base A0/D0
            label = "Relay 0";
        };
    };
};
```

## Hardware Setup

1. **Required**: Connect Grove relay module to expansion base A0/D0 port
2. **Optional**: Connect external button/switch between D1 pin and ground for momentary control
3. **Built-in**: Uses the existing XIAO button for toggle functionality