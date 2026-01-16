/*
    Controlling a strip of WS2811 LEDs that will we wrapped in a tree, allowing them to twinkle intermittenty.
   Each LED will be randomly assigned a color on each call of the pulse function. The LED's will increase and decrease
   in brightness rather than simply turning on and off. They will "pulse" in this way at a random rate (within
   certain boundaries), and only a few LED's will be on at a time.
*/

/*
   -- Twinkling Tree --

   This source code of graphical user interface
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.8 or later version
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/
     - for ANDROID 4.11.1 or later version;
     - for iOS 1.9.1 or later version;

   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__HARDSERIAL

#include <RemoteXY.h>

// RemoteXY connection settings
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 9600


// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 255 bytes
{ 255, 10, 0, 0, 0, 248, 0, 16, 27, 1, 130, 0, 255, 0, 64, 34, 175, 2, 1, 5,
  4, 10, 5, 31, 24, 24, 24, 79, 78, 0, 79, 70, 70, 0, 2, 1, 27, 4, 10, 5,
  31, 24, 24, 24, 79, 78, 0, 79, 70, 70, 0, 2, 1, 48, 4, 10, 5, 31, 24, 24,
  24, 79, 78, 0, 79, 70, 70, 0, 2, 1, 5, 18, 10, 5, 31, 24, 24, 24, 79, 78,
  0, 79, 70, 70, 0, 2, 1, 27, 17, 10, 5, 31, 24, 24, 24, 79, 78, 0, 79, 70,
  70, 0, 2, 1, 48, 17, 10, 5, 31, 24, 24, 24, 79, 78, 0, 79, 70, 70, 0, 129,
  0, 6, 10, 8, 4, 31, 82, 101, 100, 0, 129, 0, 26, 10, 12, 4, 31, 71, 114, 101,
  101, 110, 0, 129, 0, 49, 10, 8, 4, 31, 66, 108, 117, 101, 0, 129, 0, 4, 24, 12,
  4, 31, 89, 101, 108, 108, 111, 119, 0, 129, 0, 26, 24, 12, 4, 31, 80, 117, 114, 112,
  108, 101, 0, 129, 0, 48, 24, 11, 4, 31, 87, 104, 105, 116, 101, 0, 4, 128, 8, 82,
  48, 8, 31, 24, 129, 0, 14, 91, 33, 4, 31, 84, 114, 117, 110, 107, 32, 66, 114, 105,
  103, 104, 116, 110, 101, 115, 115, 0, 6, 0, 15, 38, 33, 33, 31, 26, 129, 0, 17, 73,
  27, 5, 31, 84, 114, 117, 110, 107, 32, 67, 111, 108, 111, 114, 0
};

// this structure defines all the variables and events of your control interface
struct {

  // input variables
  uint8_t switch_r; // =1 if switch ON and =0 if OFF
  uint8_t switch_g; // =1 if switch ON and =0 if OFF
  uint8_t switch_b; // =1 if switch ON and =0 if OFF
  uint8_t switch_y; // =1 if switch ON and =0 if OFF
  uint8_t switch_p; // =1 if switch ON and =0 if OFF
  uint8_t switch_w; // =1 if switch ON and =0 if OFF
  int8_t brightness; // =0..100 slider position
  uint8_t rgb_r; // =0..255 Red color value
  uint8_t rgb_g; // =0..255 Green color value
  uint8_t rgb_b; // =0..255 Blue color value

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////


#include <FastLED.h>
#define NUM_LEDS 100 // defines number of LEDs on the strip
#define DATA_PIN 6 // defines the PWM-enabled Arduino pin that will be used to send data
CRGB leds[NUM_LEDS]; // FastLED library magic

int red[3] = {255, 0, 0};
int green[3] = {0, 255, 0};
int blue[3] = {0, 0, 255};
int yellow[3] = {255, 255, 0};
int purple[3] = {255, 0, 255};
int white[3] = {255, 255, 255};
float brightness = 0.4;
int numColors = 6;
int colorPicker[6];
int index[6];

const int num_leds_on = 10; // defines the maximum number of leds that will be pulsing at any given time
int track[num_leds_on]; // keeps track of where each led is in its pulse
unsigned long previousTime[num_leds_on]; // keeps track of the last time each led was pulsed
int intensity[num_leds_on]; // intensity of each led

// define arrays of randomly generated values to be used by the pulse function
int rand_led[num_leds_on];
int rand_duration[num_leds_on];
int rand_color[num_leds_on];
int color[num_leds_on];

void pulse(int number, int color[3], int duration, int instance) { // pulse function that accepts which led to pulse, the color of the pulse, and the duration of the pulse in seconds
  unsigned long currentTime = millis(); // keeps track of the time since the start of the program (good for ~40 days before a reset is needed)

  unsigned long interval[num_leds_on];
  interval[instance] = duration;

  int check[num_leds_on][3];
  for (int j = 0; j <= 2; j++) { // creates a 3-long row vector based on what color is called. If an item in the color vector is 0, the corresponding item in the check vector is 0; if not (i.e. if it's 255), the check item is 1
    if (color[j] == 0) {
      check[instance][j] = 0;
    }
    else {
      check[instance][j] = 1;
    }
  }

  if (currentTime - previousTime[instance] >= interval[instance] && track[instance] <= 255) {
    previousTime[instance] = currentTime; // updates the previous time
    leds[number] = CRGB(check[instance][0] * (intensity[instance]), check[instance][1] * (intensity[instance]), check[instance][2] * (intensity[instance]));
    FastLED.show();
    intensity[instance] = intensity[instance] + 15; // updates the intensity
    track[instance] = track[instance] + 15; // updates the track counter
  }

  if (intensity[instance] == 270) { // when the intensity reaches 260 (above), this backs it down to 250
    intensity[instance] = 255;
  }

  if (currentTime - previousTime[instance] >= interval[instance] && track[instance] > 255 && track[instance] <= 525) {
    previousTime[instance] = currentTime;
    leds[number] = CRGB(check[instance][0] * (intensity[instance]), check[instance][1] * (intensity[instance]), check[instance][2] * (intensity[instance]));
    FastLED.show();
    intensity[instance] -= 15;
    track[instance] += 15;
  }

  if (track[instance] > 525 ) { // resets the intensity and track counter to zero once the pulse has completed
    track[instance] = 0;
    intensity[instance] = 0;
    rand_led[instance] = random(30 + 7 * instance, 37 + 7 * instance); // resets the random number and duration values after one full pulse has been completed
    rand_duration[instance] = random(5, 61);
    rand_color[instance] = random(0, 6);
  }
}

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS); // More necessary FastLED library magic
  RemoteXY_Init ();
  pinMode(DATA_PIN, OUTPUT);

  for (int i = 0; i <= 26; i++) { // turns on the trunk of the tree white
    leds[i] = CRGB(100, 100, 100);
  }
  for (int i = 27; i < 100; i++) { // sets all of the other lights to be dark initially
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();

  for (int i = 0; i < num_leds_on; i++) { // determines the initial number and duration values for all of the leds to be run at once
    rand_led[i] = random(30 + 7 * i, 37 + 7 * i);
    rand_duration[i] = random(5, 61);
    rand_color[i] = random(0, numColors);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // order of function arguments is: led number, color, pulse rate (lower values are faster), instance (0 - 49)
  RemoteXY_Handler ();
  if (RemoteXY.connect_flag) {
    color[0] = RemoteXY.rgb_r;
    color[1] = RemoteXY.rgb_g;
    color[2] = RemoteXY.rgb_b;
    brightness = RemoteXY.brightness / 100;
    numColors = RemoteXY.switch_r + RemoteXY.switch_g + RemoteXY.switch_b + RemoteXY.switch_y + RemoteXY.switch_p + RemoteXY.switch_w;

    colorPicker[0] = RemoteXY.switch_r;
    colorPicker[1] = RemoteXY.switch_g;
    colorPicker[2] = RemoteXY.switch_b;
    colorPicker[3] = RemoteXY.switch_y;
    colorPicker[4] = RemoteXY.switch_p;
    colorPicker[5] = RemoteXY.switch_w;

    for (int i = 0; i < numColors; i++) {
      for (int j = 0; j < sizeof(colorPicker); j++) {
        if (colorPicker[j]) {
          index[i] = j;
        }
      }
    }

  }

  for (int i = 0; i <= 26; i++) { // tree trunk control
    leds[i] = CRGB(color[0] * brightness, color[1] * brightness, color[2] * brightness);
  }
  FastLED.show();

  for (int i = 0;  i < num_leds_on; i++) {
    switch (rand_color[i]) {
      case 0:
        color[i] = red;
        break;
      case 1:
        color[i] = green;
        break;
      case 2:
        color[i] = blue;
        break;
      case 3:
        color[i] = yellow;
        break;
      case 4:
        color[i] = purple;
        break;
      case 5:
        color[i] = white;
        break;
    }
    pulse(rand_led[i], color[i], rand_duration[i], i);
  }
}
