#include "color_utils.h"
#include <math.h>

// Solid color palette definitions (full brightness RGB)
const RGB SOLID_COLORS[NUM_SOLID_COLORS] = {
  {255, 0, 0},     // Red
  {0, 255, 0},     // Green
  {0, 0, 255},     // Blue
  {255, 255, 0},   // Yellow
  {255, 0, 255},   // Purple
  {255, 165, 0},   // Orange
  {0, 255, 255},   // Cyan
  {255, 192, 203}, // Pink
  {255, 255, 255}  // White
};

RGB hsvToRgb(float h, float s, float v) {
  // Normalize inputs
  h = fmod(h, 360.0f); // Ensure hue is 0-360
  if (h < 0) h += 360.0f;
  s = constrain(s, 0.0f, 100.0f) / 100.0f; // Convert to 0-1
  v = constrain(v, 0.0f, 100.0f) / 100.0f; // Convert to 0-1
  
  float c = v * s; // Chroma
  float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
  float m = v - c;
  
  float r, g, b;
  
  if (h < 60) {
    r = c; g = x; b = 0;
  } else if (h < 120) {
    r = x; g = c; b = 0;
  } else if (h < 180) {
    r = 0; g = c; b = x;
  } else if (h < 240) {
    r = 0; g = x; b = c;
  } else if (h < 300) {
    r = x; g = 0; b = c;
  } else {
    r = c; g = 0; b = x;
  }
  
  RGB result;
  result.r = (uint8_t)((r + m) * 255.0f);
  result.g = (uint8_t)((g + m) * 255.0f);
  result.b = (uint8_t)((b + m) * 255.0f);
  
  return result;
}

RGB applyBrightnessAndGamma(RGB color, uint8_t brightness) {
  // Apply brightness multiplier (0-100 to 0-1.0)
  float brightnessFactor = constrain(brightness, 0, 100) / 100.0f;
  
  uint8_t r = (uint8_t)(color.r * brightnessFactor);
  uint8_t g = (uint8_t)(color.g * brightnessFactor);
  uint8_t b = (uint8_t)(color.b * brightnessFactor);
  
  // Apply gamma correction using Adafruit's built-in function
  extern Adafruit_NeoPixel strip;
  uint32_t packedColor = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  uint32_t corrected = strip.gamma32(packedColor);
  
  RGB result;
  result.r = (corrected >> 16) & 0xFF;
  result.g = (corrected >> 8) & 0xFF;
  result.b = corrected & 0xFF;
  
  return result;
}

uint8_t getEnabledColors(const uint8_t* switches, uint8_t* enabledColors) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < NUM_SOLID_COLORS; i++) {
    if (switches[i]) {
      enabledColors[count++] = i;
    }
  }
  return count;
}

bool hasEnabledColors(const uint8_t* switches) {
  for (uint8_t i = 0; i < NUM_SOLID_COLORS; i++) {
    if (switches[i]) {
      return true;
    }
  }
  return false;
}
