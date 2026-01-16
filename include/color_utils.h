#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// RGB color structure
struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// Solid color palette indices
enum SolidColor {
  COLOR_RED = 0,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW,
  COLOR_PURPLE,
  COLOR_ORANGE,
  COLOR_CYAN,
  COLOR_PINK,
  COLOR_WHITE,
  NUM_SOLID_COLORS
};

// Solid color palette (RGB values)
extern const RGB SOLID_COLORS[NUM_SOLID_COLORS];

/**
 * Convert HSV to RGB
 * @param h Hue (0-360 degrees)
 * @param s Saturation (0-100 %)
 * @param v Value/Brightness (0-100 %)
 * @return RGB structure with values 0-255
 */
RGB hsvToRgb(float h, float s, float v);

/**
 * Apply brightness multiplier and gamma correction to RGB color
 * @param color Input RGB color (0-255)
 * @param brightness Brightness multiplier (0-100)
 * @return Gamma-corrected RGB color
 */
RGB applyBrightnessAndGamma(RGB color, uint8_t brightness);

/**
 * Get list of enabled solid colors from RemoteXY switches
 * @param switches Array of 9 switch states from RemoteXY
 * @param enabledColors Output array to store enabled color indices
 * @return Number of enabled colors
 */
uint8_t getEnabledColors(const uint8_t* switches, uint8_t* enabledColors);

/**
 * Check if any solid colors are enabled
 * @param switches Array of 9 switch states from RemoteXY
 * @return true if at least one color is enabled
 */
bool hasEnabledColors(const uint8_t* switches);

#endif // COLOR_UTILS_H
