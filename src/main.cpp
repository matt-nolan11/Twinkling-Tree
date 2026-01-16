#include <Arduino.h>
#include <EEPROM.h>
#include <remoteXY_stuff.h>
#include <Adafruit_NeoPixel.h>
#include "color_utils.h"
#include "animations.h"
#include "ota_manager.h"

// ===========================
// Hardware Configuration
// ===========================

#define LED_PIN 43
#define NUM_LEDS 100
#define MIN_FRAME_INTERVAL 5  // milliseconds

// LED section boundaries
const uint8_t TRUNK_START = 0;
const uint8_t TRUNK_END = 26;
const uint8_t BRANCH_START = 27;
const uint8_t BRANCH_END = 99;

// ===========================
// Global Objects
// ===========================

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_RGB + NEO_KHZ800);

// ===========================
// Mode Tracking
// ===========================

uint8_t lastAnimationMode = 0;
uint8_t lastColorMode = 0;

// ===========================
// Setup
// ===========================

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000);  // Give Serial time to initialize
  
  // Initialize onboard LED as power indicator
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Turn on as power indicator
  
  // Initialize OTA updates (WiFi-based, non-blocking)
  Serial.println("\n=== Twinkling Tree Startup ===");
  initOTA();
  
  // Initialize RemoteXY (BLE-based)
  initializeRemoteXY();
  
  // Set default values for RemoteXY EEPROM-saved variables
  // (These will be used if EEPROM is empty/invalid on first boot)
  RemoteXY.trunk_brightness = 50;
  RemoteXY.branch_brightness = 50;
  RemoteXY.speed = 50;
  RemoteXY.branch_num_lights = 50;
  RemoteXY.animation_mode = 0;  // Fairy
  RemoteXY.color_mode = 0;      // Rainbow
  
  // Initialize EEPROM for RemoteXY (allocates memory based on flagged variables)
  EEPROM.begin(getRemoteXYEEPROMSize());
  
  // Initialize RemoteXY EEPROM system (loads saved values if valid)
  initializeRemoteXYEEPROM();
  
  // Initialize LED strip
  strip.begin();
  strip.show();
  strip.setBrightness(255);  // Brightness handled in software with strip.gamma32()
  
  // Initialize animations
  initAnimations();
  
  // Seed random number generator using micros() for randomness
  // Note: Pin 0 is not an ADC pin on ESP32-S3
  randomSeed(micros());
}

// ===========================
// Main Loop
// ===========================

void loop() {
  static unsigned long lastFrameTime = 0;
  unsigned long now = millis();
  
  // Enforce minimum frame interval
  if (now - lastFrameTime < MIN_FRAME_INTERVAL) {
    return;
  }
  lastFrameTime = now;
  
  // Handle OTA updates
  handleOTA();
  
  // Handle RemoteXY communication (automatically saves EEPROM-flagged variables when changed)
  handleRemoteXY();
  
  // Detect mode transitions and reset state
  if (RemoteXY.animation_mode != lastAnimationMode || 
      RemoteXY.color_mode != lastColorMode) {
    resetAnimationState();
    lastAnimationMode = RemoteXY.animation_mode;
    lastColorMode = RemoteXY.color_mode;
  }
  
  // Update trunk LEDs (independent of branch animations)
  updateTrunk(strip);
  
  // Update branch LEDs based on animation mode
  switch (RemoteXY.animation_mode) {
    case 0:  // Fairy
      updateFairies(strip);
      break;
    case 1:  // Wave
      updateWave(strip);
      break;
    case 2:  // Static
      updateStatic(strip);
      break;
    default:
      // Unknown mode - turn off branch LEDs
      for (int i = BRANCH_START; i <= BRANCH_END; i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
      break;
  }
  
  // Update display outputs for RemoteXY
  RemoteXY.branch_num_lights_display = RemoteXY.branch_num_lights;
  RemoteXY.speed_display = RemoteXY.speed;
  
  // Push updates to LED strip
  strip.show();
}
