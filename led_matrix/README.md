# LED Matrix with Audio & IMU Visualizations

This project features a 40x6 RGB LED matrix (5x 8x6 panels = 240 WS2812 LEDs) with interactive audio-reactive and IMU-responsive animations, plus a water simulation mode.

## 🎨 Features

### 10 Dynamic Animations
- **Audio-Reactive**: Spectrum analyzer, VU meters, bass pulse, waveform display
- **IMU-Responsive**: Gravity particles, tilt gradient, shake burst detection
- **Combined Effects**: Audio rain, bass ripples, reactive spiral
- **Classic Mode**: Interactive water simulation with tilt physics

### Hardware Integration
- **Microphone**: PDM DMIC for real-time audio capture
- **Accelerometer**: LSM6DSO 6-axis IMU for tilt and shake detection
- **Touch Sensors**: 4 capacitive touch inputs (D0-D3) for mode control
- **LED Matrix**: 5x 8x6 WS2812B panels (240 total LEDs)

## 📦 Hardware Requirements

- Seeed Studio Xiao nRF54L15
- 5x 8x6 RGB LED matrix panels (WS2812B, raster layout)
- LSM6DSO IMU (I2C on D4/D5)
- PDM DMIC microphone
- External 5V power supply (1.5A+ recommended)

## 🔌 Wiring

### LED Matrix
- Matrix DIN -> D6 (GPIO2.08)
- Matrix VCC -> 5V external power
- Matrix GND -> Common ground

### IMU (LSM6DSO)
- SDA -> D4 (I2C30 SDA)
- SCL -> D5 (I2C30 SCL)
- VCC -> 3.3V
- GND -> GND

### Microphone (PDM DMIC)
- CLK -> PDM CLK
- DATA -> PDM DATA
- VCC -> 3.3V
- GND -> GND

## 🎮 Controls

**User Button (SW0):**
- Press to cycle through all 10 animations
- Animations loop continuously

## 🎵 Animations

See [ANIMATIONS.md](ANIMATIONS.md) for detailed descriptions of all 10 animation modes:
1. Spectrum Bars - Frequency spectrum analyzer
2. VU Meter - Classic VU meters with peak hold
3. Pulse - Bass-driven expanding circles
4. Waveform - Scrolling audio waveform
5. Gravity Particles - Tilt-responsive physics
6. Tilt Gradient - Orientation-based color shifts
7. Shake Burst - Shake-triggered particle explosions
8. Audio Rain - Sound-reactive falling particles
9. Bass Ripple - Beat-driven expanding ripples
10. Reactive Spiral - Combined audio + tilt effects

Plus the original **Water Simulation** mode with realistic liquid physics!

## 🛠️ Building and Flashing

```bash
cd led_matrix
west build -b xiao_nrf54l15 -p
west flash
```

## ⚙️ Configuration

Edit `prj.conf` to adjust:
- `CONFIG_MATRIX_WIDTH`: Matrix width (default: 8)
- `CONFIG_MATRIX_HEIGHT`: Matrix height (default: 6)
- `CONFIG_SAMPLE_LED_BRIGHTNESS`: LED brightness (1-255, default: 16)
- `CONFIG_SAMPLE_LED_UPDATE_DELAY`: Frame delay in ms (default: 50)

## Power Considerations

With `CONFIG_SERIAL=n`, the device will start immediately on battery power without requiring USB connection.

Maximum power consumption at full brightness: ~48 LEDs × 60mA = 2.88A
Recommended to use external 5V power supply rated for at least 3A.
