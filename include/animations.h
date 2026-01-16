#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Ramp.h>
#include "color_utils.h"
#include "remoteXY_stuff.h"

// LED section boundaries (defined in main.cpp)
extern const uint8_t TRUNK_START;
extern const uint8_t TRUNK_END;
extern const uint8_t BRANCH_START;
extern const uint8_t BRANCH_END;

// Fairy animation
#define MAX_FAIRIES 20

struct Fairy {
  uint8_t ledIndex;
  rampInt ramp;
  RGB color;
  uint16_t duration;
  bool active;
};

// Animation state
extern Fairy fairies[MAX_FAIRIES];
extern float globalPhaseHue;
extern unsigned long lastPhaseUpdate;
extern float wavePosition;
extern unsigned long lastWaveUpdate;

// Parameter mapping
uint8_t mapFairyCount(int8_t branch_num_lights);
void mapSpeedToDuration(int8_t speed, uint16_t* minDuration, uint16_t* maxDuration);

// Animation functions
void initAnimations();
void updateTrunk(Adafruit_NeoPixel& strip);
void updateFairies(Adafruit_NeoPixel& strip);
void updateWave(Adafruit_NeoPixel& strip);
void updateStatic(Adafruit_NeoPixel& strip);
void resetAnimationState();

#endif // ANIMATIONS_H
