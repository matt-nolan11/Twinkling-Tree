# Twinkling Tree Implementation Specification

## Hardware Configuration
- **Platform**: ESP32-S3 (Seeed Xiao)
- **LED Controller**: Adafruit NeoPixel library
- **Animation Library**: RAMP library (sinusoidal easing for smooth fades)
- **Control Interface**: RemoteXY over Bluetooth LE
- **LED Layout**: 
  - Trunk section: LEDs 0-26 (27 LEDs, fixed count)
  - Branch section: LEDs 27-99 (73 LEDs)

## Control Structure

### Trunk Controls (Independent)
- **Trunk Brightness** (0-100): Overall brightness multiplier for trunk section
- **Trunk Color**: RGB color picker - sets solid color for all 27 trunk LEDs

### Branch Controls (Mode-Dependent)

#### Primary Selectors
- **Animation Mode**: Fairy / Wave / Static
- **Color Mode**: Rainbow / Phase / Solid
- **Branch Brightness** (0-100): Overall brightness multiplier for branch LEDs
- **Light Count** (0-100): Maps to 5-20 concurrent fairies (Fairy mode) or pattern density (Wave/Static)
- **Speed** (0-100): Animation speed control (higher = faster, controls timing not framerate)

#### Solid Color Palette (for Solid Color Mode)
9 toggleable colors: Red, Green, Blue, Yellow, Purple, Orange, Cyan, Pink, White

---

## Animation Mode Specifications

### **Fairy Mode**
Individual "fairies" pulse asynchronously across the branch section using RAMP library with sinusoidal easing.

**Behavior**:
- `branch_num_lights` maps to 5-20 concurrent fairies (linear mapping from 0-100 input)
- Each fairy independently:
  - Assigned random LED position in branch section (27-99)
  - Picks random pulse duration from speed-adjusted range (starting values: speed=0 → 2000-4000ms, speed=100 → 300-800ms)
  - Fades in/out using RAMP library's SINUSOIDAL_INOUT mode for smooth organic motion
  - Upon completion, respawns with new random position, duration, and color
- All timing is millis()-based, independent of frame rate
- Maintains high update rate (≥5ms frame intervals) regardless of speed setting

**Color Assignment** (per Color Mode):
- **Rainbow**: Each new fairy randomly samples from HSV spectrum (H: 0-360°, S: 100%, V: 100%)
- **Phase**: 
  - Fairies are closely spaced in hue (~5° apart)
  - Global phase hue shifts continuously over time (timer-based, speed-scaled)
  - Each fairy offset by small amount from current global phase (e.g., fairy 0 at phase, fairy 1 at phase+5°, fairy 2 at phase+10°, etc.)
  - Creates cohesive color group that slowly evolves through spectrum
- **Solid**: Each new fairy randomly picks from enabled Solid Color switches
  - If no colors enabled: all branch LEDs remain black (disabled output)

---

### **Wave Mode**
Entire branch section illuminated with animated color patterns.

**Behavior**:
- All branch LEDs (27-99) lit simultaneously
- Pattern continuously "moves" along the strip using timer-based animation (not frame-dependent)
- Speed controls how fast pattern progresses via timing intervals
- `branch_num_lights` may control wavelength or pattern density
- Maintains high update rate (≥5ms frame intervals)

**Color Patterns** (per Color Mode):
- **Rainbow**: 
  - Full HSV spectrum (0-360°) mapped across branch section length
  - Pattern slides continuously along the strip using timer-based position updates
  - Always displays complete rainbow gradient, position shifts
  
- **Phase**: 
  - Entire branch section same color at any moment
  - Color shifts through HSV spectrum over time using timer-based hue increments
  - Smooth color transitions across full spectrum
  
- **Solid**: 
  - Branch divided into segments, each segment one of the enabled Solid Colors
  - Color segments "walk" along the strip using timer-based rotation
  - Number of segments determined by count of enabled colors or `branch_num_lights`
  - If no colors enabled: all branch LEDs black (disabled output)

---

### **Static Mode**
Same color distribution as Wave mode but without movement.

**Behavior**:
- Color pattern fixed in place, no animation
- `speed` has no effect in this mode
- Pattern determined by Color Mode at initialization or mode change

**Color Patterns** (per Color Mode):
- **Rainbow**: HSV spectrum mapped across branch section, stationary
- **Phase**: Entire branch section one solid color (hue can be set or cycled on mode change)
- **Solid**: Branch divided into fixed color segments using enabled colors
  - If no colors enabled: all branch LEDs black (disabled output)

---

## Additional Implementation Notes

### Global Parameters
- **Gamma Correction**: Apply gamma correction to all RGB color outputs before sending to LEDs
- All brightness values (0-100) applied as percentage multipliers to final RGB output
- HSV to RGB conversion required for Rainbow and Phase modes
- Smooth transitions preferred when switching modes/colors

### Timing & Performance
- **Timer-based animations**: All animation timing uses `millis()` for non-blocking, frame-independent operation
- **High update rate**: Maintain ≥5ms frame intervals regardless of speed setting
- Speed controls animation timing parameters, NOT frame rate
- Never use `delay()`; if needed use `RemoteXYEngine.delay()` 
- Maintain RemoteXY responsiveness throughout all animations

### Parameter Mappings
- **Fairy Count**: `branch_num_lights` (0-100) → 5-20 concurrent fairies (linear mapping)
- **Fairy Duration Ranges** (initial values, subject to tuning):
  - Speed = 0: 2000-4000ms per pulse
  - Speed = 100: 300-800ms per pulse
  - Linear interpolation between extremes
- **Wave Animation Speed**: Map to timer intervals or position increment rates

### Edge Cases
- If no Solid Colors selected in Solid mode: all branch LEDs black (disabled output)
- If `branch_num_lights` = 0: branch section dark (0 fairies)
- Speed range: 0 = slowest practical animation (not frozen), 100 = fastest