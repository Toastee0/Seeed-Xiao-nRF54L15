# LED Matrix Audio & IMU Animations

## Overview
The LED matrix now features 10 different audio-reactive and IMU-responsive animations. All animations leverage the microphone (DMIC) and accelerometer (LSM6DSO) for dynamic, interactive effects.

## Button Controls
- **SW0 (User Button)** - Press to cycle through animations

## Animation Modes

### Audio-Reactive Animations

#### 1. Spectrum Bars
- **Description**: Classic frequency spectrum analyzer with 8 vertical bars
- **Audio**: Each bar represents a frequency band (low to high)
- **Visual**: Rainbow color gradient (red → yellow → green → cyan → blue)
- **Best for**: Seeing overall frequency content of music

#### 2. VU Meter
- **Description**: Professional VU meters with peak hold indicators
- **Audio**: Shows energy levels per frequency band
- **Visual**: Green-to-red gradient with white peak markers
- **Features**: Peak hold dots that slowly fall
- **Best for**: Classic audio visualization look

#### 3. Pulse
- **Description**: Expanding circles driven by bass
- **Audio**: Bass energy controls circle radius
- **Visual**: Cyan expanding rings from center
- **Features**: White center flash on beat detection
- **Best for**: Electronic music with strong bass

#### 4. Waveform
- **Description**: Scrolling waveform display
- **Audio**: Overall volume drives waveform amplitude
- **Visual**: Rainbow-colored wave scrolling right-to-left
- **Best for**: Seeing audio dynamics over time

### IMU-Reactive Animations

#### 5. Gravity Particles
- **Description**: Physics simulation with particles affected by tilt
- **IMU**: Tilt direction controls gravity direction
- **Visual**: Colorful particles that roll and bounce
- **Features**: Particles bounce off edges
- **Best for**: Interactive tilting and shaking

#### 6. Tilt Gradient
- **Description**: Color gradient that follows tilt direction
- **IMU**: Tilt angle determines gradient direction and colors
- **Visual**: Smooth color transitions across the matrix
- **Features**: Dynamic color palette based on orientation
- **Best for**: Smooth, ambient effects

#### 7. Shake Burst
- **Description**: Explosive particle burst on shake detection
- **IMU**: Rapid tilt changes trigger burst effect
- **Visual**: Rainbow particles explode from center
- **Features**: Particles fade as they travel outward
- **Best for**: Interactive shake detection

### Combined Audio + IMU Animations

#### 8. Audio Rain
- **Description**: Sound-reactive falling particles
- **Audio**: Volume controls particle spawn rate
- **Visual**: Rainbow particles fall with motion blur effect
- **Features**: Particles leave trails
- **Best for**: Gentle, flowing visualization

#### 9. Bass Ripple
- **Description**: Expanding ripples triggered by bass hits
- **Audio**: Beat detection triggers new ripples
- **Visual**: Colorful concentric circles expanding from center
- **Features**: Multiple overlapping ripples
- **Best for**: Music with distinct beat patterns

#### 10. Reactive Spiral
- **Description**: Rotating spiral affected by audio and tilt
- **Audio**: Bass controls spiral tightness
- **IMU**: Tilt moves the spiral's center point
- **Visual**: Rainbow spiral rotating and morphing
- **Features**: Combines audio energy with physical movement
- **Best for**: Dynamic, hypnotic effects

## Technical Features

### Audio Processing
- **Sample Rate**: 16 kHz
- **FFT Size**: 512 points
- **Frequency Bands**: 8 mel-scale bands
- **Beat Detection**: Automatic bass-driven beat detection
- **Smoothing**: Temporal smoothing for stable visualization

### IMU Integration
- **Sensor**: LSM6DSO 6-axis IMU
- **Tilt Range**: ±90° for roll and pitch
- **Update Rate**: Real-time (every frame)
- **Features**: Shake detection, gravity simulation

### Performance
- **Frame Rate**: ~50 FPS (20ms update interval)
- **LED Brightness**: Configurable (default: safe for 5 matrices)
- **Power Budget**: Optimized for 1.5A @ 5V

## Code Structure

```
led_matrix/src/
├── main.c           - Main loop, mode switching, touch handling
├── audio_viz.c/h    - Audio capture, FFT, mel-spectrogram
├── animations.c/h   - All animation implementations
├── water_physics.c/h - Original water simulation
└── font_5x5.h       - Text rendering
```

## Building and Flashing

```bash
cd led_matrix
west build -b xiao_nrf54l15 -p
west flash
```

## Usage Tips

1. **Start in Animation Mode**: Device boots into animation mode with audio active
2. **Try Different Music**: Each animation responds differently to various music genres
3. **Tilt and Shake**: IMU animations respond to physical movement
4. **Switch Modes**: Use D0 to toggle back to water simulation
5. **Explore Animations**: Use D1/D2 to cycle through all 10 animations

## Future Enhancements

Potential additions:
- Custom color palettes (via D3 button)
- Adjustable sensitivity/brightness
- Animation speed control
- OLED display integration for mode names
- Bluetooth audio streaming
- More complex particle systems
- User-defined animation sequences
