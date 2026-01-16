#include "animations.h"

// Animation state
Fairy fairies[MAX_FAIRIES];
float globalPhaseHue = 0.0f;
unsigned long lastPhaseUpdate = 0;
float wavePosition = 0.0f;
unsigned long lastWaveUpdate = 0;

// ===========================
// Parameter Mapping Functions
// ===========================

uint8_t mapFairyCount(int8_t branch_num_lights) {
  return constrain(map(branch_num_lights, 0, 100, 5, 20), 5, 20);
}

void mapSpeedToDuration(int8_t speed, uint16_t* minDuration, uint16_t* maxDuration) {
  *minDuration = map(speed, 0, 100, 2000, 300);
  *maxDuration = map(speed, 0, 100, 4000, 800);
}

// ===========================
// Initialization
// ===========================

void initAnimations() {
  for (uint8_t i = 0; i < MAX_FAIRIES; i++) {
    fairies[i].active = false;
  }
}

void resetAnimationState() {
  for (uint8_t i = 0; i < MAX_FAIRIES; i++) {
    fairies[i].active = false;
  }
  wavePosition = 0.0f;
}

// ===========================
// Trunk Control
// ===========================

void updateTrunk(Adafruit_NeoPixel& strip) {
  RGB trunkColor = {RemoteXY.trunk_r, RemoteXY.trunk_g, RemoteXY.trunk_b};
  RGB finalColor = applyBrightnessAndGamma(trunkColor, RemoteXY.trunk_brightness);
  
  for (int i = TRUNK_START; i <= TRUNK_END; i++) {
    strip.setPixelColor(i, finalColor.r, finalColor.g, finalColor.b);
  }
}

// ===========================
// Fairy Animation Mode
// ===========================

RGB getFairyColor(uint8_t fairyIndex) {
  RGB color;
  
  switch (RemoteXY.color_mode) {
    case 0: // Rainbow - random HSV sampling
      color = hsvToRgb(random(0, 360), 100, 100);
      break;
      
    case 1: // Phase - global phase + offset
      {
        float hue = globalPhaseHue + (fairyIndex * 5.0f);
        if (hue >= 360.0f) hue -= 360.0f;
        color = hsvToRgb(hue, 100, 100);
      }
      break;
      
    case 2: // Solid - pick from enabled colors
      {
        uint8_t switches[NUM_SOLID_COLORS] = {
          RemoteXY.switch_red, RemoteXY.switch_green, RemoteXY.switch_blue,
          RemoteXY.switch_yellow, RemoteXY.switch_purple, RemoteXY.switch_orange,
          RemoteXY.switch_cyan, RemoteXY.switch_pink, RemoteXY.switch_white
        };
        
        uint8_t enabledColors[NUM_SOLID_COLORS];
        uint8_t count = getEnabledColors(switches, enabledColors);
        
        if (count > 0) {
          uint8_t colorIndex = enabledColors[random(0, count)];
          color = SOLID_COLORS[colorIndex];
        } else {
          color = {0, 0, 0};
        }
      }
      break;
      
    default:
      color = {255, 255, 255};
      break;
  }
  
  return color;
}

void spawnFairy(uint8_t index) {
  if (index >= MAX_FAIRIES) return;
  
  fairies[index].ledIndex = random(BRANCH_START, BRANCH_END + 1);
  fairies[index].color = getFairyColor(index);
  
  uint16_t minDur, maxDur;
  mapSpeedToDuration(RemoteXY.speed, &minDur, &maxDur);
  fairies[index].duration = random(minDur, maxDur + 1);
  
  fairies[index].ramp.go(255, fairies[index].duration, SINUSOIDAL_INOUT, ONCEFORWARD);
  fairies[index].active = true;
}

void updateFairies(Adafruit_NeoPixel& strip) {
  // Check if using Solid mode with no colors enabled
  if (RemoteXY.color_mode == 2) {
    uint8_t switches[NUM_SOLID_COLORS] = {
      RemoteXY.switch_red, RemoteXY.switch_green, RemoteXY.switch_blue,
      RemoteXY.switch_yellow, RemoteXY.switch_purple, RemoteXY.switch_orange,
      RemoteXY.switch_cyan, RemoteXY.switch_pink, RemoteXY.switch_white
    };
    
    if (!hasEnabledColors(switches)) {
      for (int i = BRANCH_START; i <= BRANCH_END; i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
      return;
    }
  }
  
  // Update global phase for Phase color mode
  if (RemoteXY.color_mode == 1) {
    unsigned long now = millis();
    if (now - lastPhaseUpdate > 50) {
      float phaseSpeed = map(RemoteXY.speed, 0, 100, 5, 100) / 100.0f;
      globalPhaseHue += phaseSpeed;
      if (globalPhaseHue >= 360.0f) globalPhaseHue -= 360.0f;
      lastPhaseUpdate = now;
    }
  }
  
  // Clear branch section
  for (int i = BRANCH_START; i <= BRANCH_END; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  
  // Update fairies
  uint8_t targetCount = mapFairyCount(RemoteXY.branch_num_lights);
  
  for (uint8_t i = 0; i < targetCount; i++) {
    if (!fairies[i].active) {
      spawnFairy(i);
    }
    
    fairies[i].ramp.update();
    uint8_t intensity = fairies[i].ramp.getValue();
    
    if (fairies[i].ramp.isFinished()) {
      spawnFairy(i);
    }
    
    // Apply intensity and brightness
    RGB finalColor = fairies[i].color;
    finalColor.r = (finalColor.r * intensity) / 255;
    finalColor.g = (finalColor.g * intensity) / 255;
    finalColor.b = (finalColor.b * intensity) / 255;
    finalColor = applyBrightnessAndGamma(finalColor, RemoteXY.branch_brightness);
    
    // Additive blending
    uint32_t existingColor = strip.getPixelColor(fairies[i].ledIndex);
    uint8_t existing_r = (existingColor >> 16) & 0xFF;
    uint8_t existing_g = (existingColor >> 8) & 0xFF;
    uint8_t existing_b = existingColor & 0xFF;
    
    uint8_t new_r = min(255, existing_r + finalColor.r);
    uint8_t new_g = min(255, existing_g + finalColor.g);
    uint8_t new_b = min(255, existing_b + finalColor.b);
    
    strip.setPixelColor(fairies[i].ledIndex, new_r, new_g, new_b);
  }
  
  // Deactivate extra fairies
  for (uint8_t i = targetCount; i < MAX_FAIRIES; i++) {
    fairies[i].active = false;
  }
}

// ===========================
// Wave Animation Mode
// ===========================

void updateWave(Adafruit_NeoPixel& strip) {
  unsigned long now = millis();
  const uint8_t BRANCH_LENGTH = BRANCH_END - BRANCH_START + 1;
  
  // Update wave position
  if (now - lastWaveUpdate > 20) {
    float waveSpeed = map(RemoteXY.speed, 0, 100, 10, 500) / 1000.0f;
    wavePosition += waveSpeed;
    if (wavePosition >= 360.0f) wavePosition -= 360.0f;
    lastWaveUpdate = now;
  }
  
  switch (RemoteXY.color_mode) {
    case 0: // Rainbow - sliding spectrum
      for (uint8_t i = 0; i < BRANCH_LENGTH; i++) {
        float hue = (i * 360.0f / BRANCH_LENGTH) + wavePosition;
        if (hue >= 360.0f) hue -= 360.0f;
        
        RGB color = hsvToRgb(hue, 100, 100);
        RGB finalColor = applyBrightnessAndGamma(color, RemoteXY.branch_brightness);
        strip.setPixelColor(BRANCH_START + i, finalColor.r, finalColor.g, finalColor.b);
      }
      break;
      
    case 1: // Phase - uniform shifting color
      {
        RGB color = hsvToRgb(globalPhaseHue, 100, 100);
        RGB finalColor = applyBrightnessAndGamma(color, RemoteXY.branch_brightness);
        
        for (int i = BRANCH_START; i <= BRANCH_END; i++) {
          strip.setPixelColor(i, finalColor.r, finalColor.g, finalColor.b);
        }
      }
      break;
      
    case 2: // Solid - walking segments
      {
        uint8_t switches[NUM_SOLID_COLORS] = {
          RemoteXY.switch_red, RemoteXY.switch_green, RemoteXY.switch_blue,
          RemoteXY.switch_yellow, RemoteXY.switch_purple, RemoteXY.switch_orange,
          RemoteXY.switch_cyan, RemoteXY.switch_pink, RemoteXY.switch_white
        };
        
        uint8_t enabledColors[NUM_SOLID_COLORS];
        uint8_t count = getEnabledColors(switches, enabledColors);
        
        if (count == 0) {
          for (int i = BRANCH_START; i <= BRANCH_END; i++) {
            strip.setPixelColor(i, 0, 0, 0);
          }
        } else {
          uint8_t segmentSize = BRANCH_LENGTH / count;
          
          for (uint8_t i = 0; i < BRANCH_LENGTH; i++) {
            uint8_t segmentIndex = ((i + (int)wavePosition) / max(1, (int)segmentSize)) % count;
            RGB color = SOLID_COLORS[enabledColors[segmentIndex]];
            RGB finalColor = applyBrightnessAndGamma(color, RemoteXY.branch_brightness);
            strip.setPixelColor(BRANCH_START + i, finalColor.r, finalColor.g, finalColor.b);
          }
        }
      }
      break;
  }
  
  // Update global phase for Phase mode
  if (RemoteXY.color_mode == 1) {
    if (now - lastPhaseUpdate > 50) {
      float phaseSpeed = map(RemoteXY.speed, 0, 100, 5, 100) / 100.0f;
      globalPhaseHue += phaseSpeed;
      if (globalPhaseHue >= 360.0f) globalPhaseHue -= 360.0f;
      lastPhaseUpdate = now;
    }
  }
}

// ===========================
// Static Animation Mode
// ===========================

void updateStatic(Adafruit_NeoPixel& strip) {
  const uint8_t BRANCH_LENGTH = BRANCH_END - BRANCH_START + 1;
  
  switch (RemoteXY.color_mode) {
    case 0: // Rainbow - static spectrum
      for (uint8_t i = 0; i < BRANCH_LENGTH; i++) {
        float hue = i * 360.0f / BRANCH_LENGTH;
        
        RGB color = hsvToRgb(hue, 100, 100);
        RGB finalColor = applyBrightnessAndGamma(color, RemoteXY.branch_brightness);
        strip.setPixelColor(BRANCH_START + i, finalColor.r, finalColor.g, finalColor.b);
      }
      break;
      
    case 1: // Phase - single solid color
      {
        RGB color = hsvToRgb(globalPhaseHue, 100, 100);
        RGB finalColor = applyBrightnessAndGamma(color, RemoteXY.branch_brightness);
        
        for (int i = BRANCH_START; i <= BRANCH_END; i++) {
          strip.setPixelColor(i, finalColor.r, finalColor.g, finalColor.b);
        }
      }
      break;
      
    case 2: // Solid - fixed segments
      {
        uint8_t switches[NUM_SOLID_COLORS] = {
          RemoteXY.switch_red, RemoteXY.switch_green, RemoteXY.switch_blue,
          RemoteXY.switch_yellow, RemoteXY.switch_purple, RemoteXY.switch_orange,
          RemoteXY.switch_cyan, RemoteXY.switch_pink, RemoteXY.switch_white
        };
        
        uint8_t enabledColors[NUM_SOLID_COLORS];
        uint8_t count = getEnabledColors(switches, enabledColors);
        
        if (count == 0) {
          for (int i = BRANCH_START; i <= BRANCH_END; i++) {
            strip.setPixelColor(i, 0, 0, 0);
          }
        } else {
          uint8_t segmentSize = BRANCH_LENGTH / count;
          
          for (uint8_t i = 0; i < BRANCH_LENGTH; i++) {
            uint8_t segmentIndex = (i / max(1, (int)segmentSize)) % count;
            RGB color = SOLID_COLORS[enabledColors[segmentIndex]];
            RGB finalColor = applyBrightnessAndGamma(color, RemoteXY.branch_brightness);
            strip.setPixelColor(BRANCH_START + i, finalColor.r, finalColor.g, finalColor.b);
          }
        }
      }
      break;
  }
}
