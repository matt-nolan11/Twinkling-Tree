#pragma once

/*
   -- Twinkling Tree --
   
   RemoteXY GUI interface declarations
   Full implementation in src/remoteXY_implementation.cpp
*/

#include <Arduino.h>

// RemoteXY structure definition
struct RemoteXY_Struct {
  // input variables
  uint8_t switch_red;
  uint8_t switch_green;
  uint8_t switch_blue;
  uint8_t switch_yellow;
  uint8_t switch_purple;
  uint8_t switch_orange;
  int8_t trunk_brightness;
  uint8_t trunk_r;
  uint8_t trunk_g;
  uint8_t trunk_b;
  int8_t branch_num_lights;
  uint8_t switch_cyan;
  uint8_t switch_pink;
  uint8_t switch_white;
  uint8_t color_mode;
  int8_t branch_brightness;
  int8_t speed;
  uint8_t animation_mode;

  // output variables
  int8_t branch_num_lights_display;
  int8_t speed_display;
};

// External declarations (defined in remoteXY_implementation.cpp)
extern RemoteXY_Struct RemoteXY;
extern const uint8_t RemoteXY_CONF_PROGMEM[];

// Forward declare RemoteXY engine class
class CRemoteXY;
extern CRemoteXY RemoteXYEngine;

// RemoteXY functions (implemented in remoteXY_stuff.cpp)
void initializeRemoteXY();
void handleRemoteXY();
void initializeRemoteXYEEPROM();
uint16_t getRemoteXYEEPROMSize();