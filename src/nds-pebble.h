#pragma once

#include "pebble.h"

static GColor bottomColors[16];
static GColor middleColors[16];
static GColor topColors[16];

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  //GColor BackgroundColor;
  //GColor ForegroundColor;
  bool SecondTick;
  //bool Animations;
  int FavColor;
} ClaySettings;

#define NUM_CLOCK_TICKS 9

static const struct GPathInfo ANALOG_BG_POINTS[] = {
  { 4,
    (GPoint []) {
      {69, 10},
      {74, 10},
      {74, 13},
      {69, 13}
    }
  },
  { 4, (GPoint []){
    {85, 26},
    {90, 26},
    {90, 29},
    {85, 29}
  }
  },
  { 4, (GPoint []){
    {85, 70},
    {90, 70},
    {90, 73},
    {85, 73}
  }
  },
  { 4, (GPoint []){
    {69, 86},
    {74, 86},
    {74, 89},
    {69, 89}
  }
  },
  { 4, (GPoint []){
    {25, 86},
    {30, 86},
    {30, 89},
    {25, 89}
  }
  },
  { 4, (GPoint []){
    {9, 70},
    {14, 70},
    {14, 73},
    {9, 73}
  }
  },
  { 4, (GPoint []){
    {9, 26},
    {14, 26},
    {14, 29},
    {9, 29}
  }
  },
  { 4, (GPoint []){
    {25, 10},
    {30, 10},
    {30, 13},
    {25, 13}
  }
  },
  { 4, (GPoint []){
    {46, 47},
    {52, 47},
    {52, 51},
    {46, 51}
  }
  },
};

static const struct GPathInfo ANALOG_BG_POINTS_EMERY[] = {
  { 4,
    (GPoint []) {
      {104, 16},
      {111, 16},
      {111, 21},
      {104, 21}
    }
  },
  { 4, (GPoint []){
    {128, 40},
    {135, 40},
    {135, 45},
    {128, 45}
  }
  },
  { 4, (GPoint []){
    {128, 106},
    {135, 106},
    {135, 111},
    {128, 111}
  }
  },
  { 4, (GPoint []){
    {104, 130},
    {111, 130},
    {111, 135},
    {104, 135}
  }
  },
  { 4, (GPoint []){
    {38, 130},
    {45, 130},
    {45, 135},
    {38, 135}
  }
  },
  { 4, (GPoint []){
    {14, 106},
    {21, 106},
    {21, 111},
    {14, 111}
  }
  },
  { 4, (GPoint []){
    {14, 40},
    {21, 40},
    {21, 45},
    {14, 45}
  }
  },
  { 4, (GPoint []){
    {38, 16},
    {45, 16},
    {45, 21},
    {38, 21}
  }
  },
  { 4, (GPoint []){
    {70, 71},
    {79, 71},
    {79, 78},
    {70, 78}
  }
  },
};

/*static const GPathInfo MINUTE_HAND_POINTS = {
 *  3, (GPoint []) {
 *    { -8, 20 },
 *    { 8, 20 },
 *    { 0, -80 }
 *  }
 * };*/

/*static const GPathInfo HOUR_HAND_POINTS = {
 *  3, (GPoint []){
 *    {-6, 20},
 *    {6, 20},
 *    {0, -60}
 *  }
 * };*/
