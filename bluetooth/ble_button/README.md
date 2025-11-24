# BLE Button Sample for XIAO nRF54L15

This sample demonstrates a Bluetooth Low Energy beacon that updates its advertising data based on button presses. Each time the user button is pressed, a counter is incremented and broadcast in the manufacturer-specific data of the advertising packet.

## Features

- BLE beacon advertising with manufacturer-specific data
- Button press counter transmitted in advertising packet  
- LED feedback for button presses and running status
- Direct GPIO control (no DK library dependency)
- Adapted from Nordic Academy BLE training exercise 2

## Hardware Requirements

- Seeed Studio XIAO nRF54L15
- User button (sw0 alias - built into board)
- LED (led0 alias - built into board)

## Functionality

1. **Advertising**: Broadcasts device name, URL, and manufacturer data containing button press count
2. **Button Press**: Each press increments counter and updates advertising data
3. **LED Feedback**: 
   - Brief flash when button is pressed
   - Continuous blink to show running status
4. **Manufacturer Data**: Contains company ID (0x0059) and 16-bit press counter

## Building

Build the sample for XIAO nRF54L15:

```bash
cd m:\nRF54L15\ble_button
west build -b xiao_nrf54l15/nrf54l15/cpuapp --pristine
```

## Flashing

Flash to the board:

```bash
west flash
```

## Usage

1. Flash the sample to your XIAO nRF54L15
2. Open a BLE scanner app on your phone/computer
3. Look for device named "XIAO_Button"
4. Press the user button on the board
5. Watch the manufacturer data change in the BLE scanner
6. LED will flash briefly on each button press and blink continuously to show running status

## Advertising Data Structure

The manufacturer-specific data contains:
- **Company ID**: 0x0059 (2 bytes)
- **Button Counter**: 16-bit counter incremented on each press (2 bytes)

Total manufacturer data payload: 4 bytes

## Expected Output

Console output will show:
```
[00:00:00.123,456] <inf> BLE_Button_Xiao: Starting BLE Button Sample for XIAO nRF54L15
[00:00:00.234,567] <inf> BLE_Button_Xiao: Set up button at GPIO_1 pin 3
[00:00:00.345,678] <inf> BLE_Button_Xiao: Bluetooth initialized
[00:00:00.456,789] <inf> BLE_Button_Xiao: Advertising successfully started
[00:00:00.567,890] <inf> BLE_Button_Xiao: Press button to increment counter in advertising data
[00:00:05.123,456] <inf> BLE_Button_Xiao: Button pressed! Count: 1
[00:00:07.234,567] <inf> BLE_Button_Xiao: Button pressed! Count: 2
```

## Technical Notes

- Uses GPIO interrupts for responsive button handling
- Advertising data is updated dynamically without stopping/restarting advertising
- Button is active-low with internal pull-up
- LED provides immediate visual feedback for button presses
- Company ID 0x0059 is Nordic Semiconductor's assigned ID (for educational use)