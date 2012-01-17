// (C)2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Sleevepeek
//
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "mode_sleevepeek.h"


// Put our resources in an easy to find list
static const PulseResource modeSleevepeekWatchDigits[] = {
  IMAGE_MODE_SLEEVEPEEK_0,
  IMAGE_MODE_SLEEVEPEEK_1,
  IMAGE_MODE_SLEEVEPEEK_2,
  IMAGE_MODE_SLEEVEPEEK_3,
  IMAGE_MODE_SLEEVEPEEK_4,
  IMAGE_MODE_SLEEVEPEEK_5,
  IMAGE_MODE_SLEEVEPEEK_6,
  IMAGE_MODE_SLEEVEPEEK_7,
  IMAGE_MODE_SLEEVEPEEK_8,
  IMAGE_MODE_SLEEVEPEEK_9,
  IMAGE_MODE_SLEEVEPEEK_COLON,
};

static const PulseResource modeSleevepeekMonth[] = {
  IMAGE_MODE_SLEEVEPEEK_JAN,
  IMAGE_MODE_SLEEVEPEEK_FEB,
  IMAGE_MODE_SLEEVEPEEK_MAR,
  IMAGE_MODE_SLEEVEPEEK_APR,
  IMAGE_MODE_SLEEVEPEEK_MAY,
  IMAGE_MODE_SLEEVEPEEK_JUN,
  IMAGE_MODE_SLEEVEPEEK_JUL,
  IMAGE_MODE_SLEEVEPEEK_AUG,
  IMAGE_MODE_SLEEVEPEEK_SEP,
  IMAGE_MODE_SLEEVEPEEK_OCT,
  IMAGE_MODE_SLEEVEPEEK_NOV,
  IMAGE_MODE_SLEEVEPEEK_DEC,
};

void mode_sleevepeek_draw_time_digit(int iPosition, int iDigit, int iRow) {
  multi_debug("time digit %i %i %i\n", iPosition, iDigit, iRow);

  pulse_draw_image(modeSleevepeekWatchDigits[iDigit], 
    SCREEN_WIDTH - 18 - (20*iRow), 42 + (9 * iPosition));
}

void mode_sleevepeek_draw_date_digit(int iPosition, int iDigit) {
  multi_debug("date digit %i %i\n", iPosition, iDigit);

}

void mode_sleevepeek_display_time() {
  multi_debug("mode_displaysleep_tick_tock\n");

  mode_sleevepeek_draw_time_digit(0, multiTimeNow.tm_hour / 10, 0);//Digit *?:??
  mode_sleevepeek_draw_time_digit(1, multiTimeNow.tm_hour % 10, 0);//Digit ?*:??
  mode_sleevepeek_draw_time_digit(2, 10, 0); // Digit ??*?? the colon
  mode_sleevepeek_draw_time_digit(3, multiTimeNow.tm_min / 10, 0);// Digit ??:*?
  mode_sleevepeek_draw_time_digit(4, multiTimeNow.tm_min % 10, 0);// Digit ??:?*

  mode_sleevepeek_draw_time_digit(0, multiTimeNow.tm_mday / 10, 1);
  mode_sleevepeek_draw_time_digit(1, multiTimeNow.tm_mday % 10, 1);
  pulse_draw_image(modeSleevepeekMonth[multiTimeNow.tm_mon], 
    SCREEN_WIDTH - 42, 63); // Month
}

// The main init function
void mode_sleevepeek_watch_functions(const enum multi_function_table iFunc,
                                       ...) {
  multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case BUTTONWAKE:
      mode_sleevepeek_display_time();
      break;
    default: // ignore features we do not use
      break;
  }
}

