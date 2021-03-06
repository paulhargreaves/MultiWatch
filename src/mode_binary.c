// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Binary watch
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_binary.h"

#define MODE_BINARY_POWERDOWN_TIME 14000

void mode_binary_display_led(int iXPos, int iYPos, bool iLit) {
  #define xLen 10 
  #define yLen 15
  #define xStart 5
  #define yStart 22
  #define xGap 15 // space between boxes on same row
  #define yGap 35
  color24_t colourWanted = { 0xFF, 0x00, 0x00, 0x00 }; // RED, off
  if (iLit) {
    colourWanted = COLOR_WHITE24; // WHITE, on
  }
  multi_draw_box(SCREEN_WIDTH - (xStart + (iXPos * xGap) + xLen),  //x1
                        yStart + (iYPos * yGap),  //y1
                        xLen, yLen, colourWanted); 
  multi_debug("display: %i %i %i\n", iXPos, iYPos, iLit);
}

void mode_binary_draw_watch_face() {
  multi_debug("hour %i min %i\n", multiTimeNow.tm_hour, multiTimeNow.tm_min);

  int i;
  for(i=0; i<6; i++) { // hours
    mode_binary_display_led(i, 0, multiTimeNow.tm_hour % 2);
    multiTimeNow.tm_hour=multiTimeNow.tm_hour / 2;
  }
  for(i=0; i<6; i++) { // minutes
    mode_binary_display_led(i, 1, multiTimeNow.tm_min % 2);
    multiTimeNow.tm_min=multiTimeNow.tm_min / 2;
  }
  for(i=0; i<6; i++) { // seconds
    mode_binary_display_led(i, 2, multiTimeNow.tm_sec % 2);
    multiTimeNow.tm_sec=multiTimeNow.tm_sec / 2;
  }
   
}

void mode_binary_watch_functions(const enum multi_function_table iFunc) {
  //multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case MODEINIT:
      multi_update_power_down_timer(MODE_BINARY_POWERDOWN_TIME);
      break;
    case MAINLOOP:
      mode_binary_draw_watch_face();
      break;
    default: // ignore features we do not use
      break;
  }
}

