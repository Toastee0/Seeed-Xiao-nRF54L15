# LED Matrix Power Budget Analysis

## Measured Data
- **Matrix size**: 8x6 = 48 LEDs
- **LED spec**: 5mA per channel (R, G, B)
- **Max power per LED**: 15mA (all channels at 255)
- **Measured at brightness 255**: 696mA for one matrix
- **Power per matrix per brightness unit**: 696mA / 255 = 2.73mA per brightness level

## Power Budget: 1.5A (1500mA)

| Brightness | mA per Matrix | Max Matrices | Total LEDs | Notes |
|------------|---------------|--------------|------------|-------|
| 5          | 14           | 107          | 5,136      | Ultra dim, max coverage |
| 30         | 81           | 18           | 864        | Very dim |
| 55         | 150          | 10           | 480        | Dim |
| 80         | 218          | 6            | 288        | Low-medium |
| 105        | 287          | 5            | 240        | Medium-low |
| 130        | 355          | 4            | 192        | Medium |
| 155        | 423          | 3            | 144        | Medium-high |
| 180        | 491          | 3            | 144        | High-medium |

> **Note**: Brightness above 200 ignored - too bright for practical use. Real-world content rarely displays full white.

## Recommended Configuration: 5-Matrix Linear Display

### Display Specifications
- **Total size**: 40×6 pixels (5 matrices in a row)
- **Total LEDs**: 240 LEDs
- **Total power at brightness 105**: 1435mA
- **Safety margin**: 65mA (4.3% headroom)

### Power Distribution
```
Matrix 1: 287mA
Matrix 2: 287mA  
Matrix 3: 287mA
Matrix 4: 287mA
Matrix 5: 287mA
─────────────────
Total:   1435mA (within 1500mA USB budget)
```

### Brightness Headroom for 5 Matrices
| Brightness | Total Power | Headroom | Status |
|------------|-------------|----------|--------|
| 80         | 1090mA     | 410mA    | ✓ Safe |
| 105        | 1435mA     | 65mA     | ✓ Recommended |
| 130        | 1775mA     | -275mA   | ✗ Over budget |
| 155        | 2115mA     | -615mA   | ✗ Over budget |

### Practical Usage for 5-Matrix Setup
- **Brightness 105** is the sweet spot
- **Typical content** (not full white): ~70% power = 1004mA actual
- **Scrolling text**: Even lower power draw
- **Animations**: Power varies with content

### Real-World Content Power Usage
Since most content isn't full white:
- **Text on black**: ~20-30% of max = 287-430mA total
- **Graphics/patterns**: ~40-60% of max = 574-860mA total  
- **Full color video**: ~70-80% of max = 1004-1148mA total

## Display Layout
```
┌─────┬─────┬─────┬─────┬─────┐
│  1  │  2  │  3  │  4  │  5  │  Each: 8×6 pixels
│ 8×6 │ 8×6 │ 8×6 │ 8×6 │ 8×6 │  Total: 40×6 pixels
└─────┴─────┴─────┴─────┴─────┘
   →→→→→→→ Data flow →→→→→→→
```

## Calculation Formulas

```
Power per matrix at brightness B = (696mA / 255) × B = 2.73mA × B
Max matrices at brightness B = floor(1500mA / (2.73mA × B))
Total LEDs = Max matrices × 48
```

## Linear Power Scaling

The power consumption scales linearly with brightness:
- **At 50% brightness (127)**: ~347mA per matrix → 4 matrices max
- **At 25% brightness (64)**: ~175mA per matrix → 8 matrices max
- **At 41% brightness (105)**: ~287mA per matrix → 5 matrices max ✓ **RECOMMENDED**
