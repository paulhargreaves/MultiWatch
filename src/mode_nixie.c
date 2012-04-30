// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// NIXIE
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "mode_nixie.h"


// Put our resources in an easy to find list
static const PulseResource nixieWatchDigits[] = {
  IMAGE_MODE_NIXIE_0,
  IMAGE_MODE_NIXIE_1,
  IMAGE_MODE_NIXIE_2,
  IMAGE_MODE_NIXIE_3,
  IMAGE_MODE_NIXIE_4,
  IMAGE_MODE_NIXIE_5,
  IMAGE_MODE_NIXIE_6,
  IMAGE_MODE_NIXIE_7,
  IMAGE_MODE_NIXIE_8,
  IMAGE_MODE_NIXIE_9,
  IMAGE_MODE_NIXIE_10,
};
  
bool modeNixieSecondsMode;

void mode_nixie_watch_functions(const enum multi_function_table iFunc, ...) {
  multi_debug("enum %i\n", iFunc);
  // varargs fuctionality is removed from this example for space reasons
  // see the powerpoint or demo.c for better examples
  switch (iFunc) {
    case BUTTONWAKE:
      mode_nixie_woken_by_button();
      break;
    case MODEINIT:
      modeNixieSecondsMode = false; // back into hours mode
      break;
    case MAINLOOP:
      mode_nixie_draw_watch_face();
      break;
    case BUTTONUP:
      modeNixieSecondsMode = !modeNixieSecondsMode;
      break;
    default: // ignore features we do not use
      break;
  }
}

int modeNixieCurrentlyDisplayed[MODE_NIXIE_DIGIT_POSITION_SIZE]; 

void mode_nixie_woken_by_button() {
  // Just woken up - display is blank
  for (int i=0; i<MODE_NIXIE_DIGIT_POSITION_SIZE; i++) {
    modeNixieCurrentlyDisplayed[i]=NIXIE_NO_DIGIT_DISPLAYED;
  }
}


void mode_nixie_draw_single_digit(int iDigitWanted,
             enum nixie_digit_position iDigitPosition, int iDotWanted) {
  multi_debug("draw_single_digit %i %i %i\n", iDigitWanted, iDigitPosition, 
             iDotWanted);

  // Already displaying this image in this place? Do nothing
  if(modeNixieCurrentlyDisplayed[iDigitPosition] == iDigitWanted + iDotWanted) {
    multi_debug("skipping draw\n");
    return; 
  }
  // Not displayed yet - store it anyway (dot causes it to shift...)
  modeNixieCurrentlyDisplayed[iDigitPosition] = iDigitWanted + iDotWanted;

  // Decide on x&y positions
  unsigned int xPos, yPos;
  xPos = 0; yPos = 0;
  switch (iDigitPosition) {
    case NIXIE_DIGIT_TOPLEFT:
      break; // stays at 0,0
    case NIXIE_DIGIT_TOPRIGHT:
      xPos = SCREEN_WIDTH / 2; break; // ypos=0
    case NIXIE_DIGIT_BOTTOMLEFT:
      yPos = SCREEN_HEIGHT / 2; break; // xpos=0
    case NIXIE_DIGIT_BOTTOMRIGHT:
      xPos = SCREEN_WIDTH / 2; yPos = SCREEN_HEIGHT / 2; break;
  }

  pulse_draw_image(nixieWatchDigits[iDigitWanted], xPos, yPos);

  if ( iDotWanted ) {
    pulse_draw_image(IMAGE_MODE_NIXIE_DOT, 78, 116);
  }

  multi_debug("draw should have happened\n");
}

void mode_nixie_draw_watch_face() {
  multi_debug("time %i:%i:%i\n", multiTimeNow.tm_hour, multiTimeNow.tm_min,
              multiTimeNow.tm_sec);
  multi_debug("date Day%i: Mon%i: Year%i Wday:%i YDay:%i\n",
              multiTimeNow.tm_mday, multiTimeNow.tm_mon, multiTimeNow.tm_year,
              multiTimeNow.tm_wday, multiTimeNow.tm_yday);

  // Hack the seconds over the hours if in seconds mode
  if ( modeNixieSecondsMode == true ) {
    multiTimeNow.tm_hour = multiTimeNow.tm_sec; 
  }
  // Top left is the 1 in 12:34
  mode_nixie_draw_single_digit(multiTimeNow.tm_hour / 10, NIXIE_DIGIT_TOPLEFT,
                                 NIXIE_NO_DIGIT_DOT);
  // Top right is the 2 in 12:34
  mode_nixie_draw_single_digit(multiTimeNow.tm_hour % 10, NIXIE_DIGIT_TOPRIGHT,
                                 NIXIE_NO_DIGIT_DOT);
  
  // Display the minutes always

  //Bottom left is the 3 in 12:34
  mode_nixie_draw_single_digit(multiTimeNow.tm_min / 10, NIXIE_DIGIT_BOTTOMLEFT,
                               NIXIE_NO_DIGIT_DOT);
  // Bottom right is the 4 in 12:34
  mode_nixie_draw_single_digit(multiTimeNow.tm_min % 10, 
                      NIXIE_DIGIT_BOTTOMRIGHT, multiTimeNow.tm_sec % 2);
}

