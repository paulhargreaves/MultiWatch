// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Display sleep
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "mode_displaysleep.h"


// SPECIAL NOTES
// We cannot use multi_register_timer at all in this mode; if we do then the
// timers will either get paused or cancelled. So, unusually, use
// the normal pulse_register and cancel functions. 


int32_t modeDisplaysleepPowerDownTimerID = -1; // safe to set here
int32_t modeDisplaysleepingTimerID = -1;

#define MODE_DISPLAYSLEEP_DIGIT_POS_SIZE 5 // 4 digits and a colon
#define MODE_DISPLAYSLEEP_NO_DIGIT 99

int modeDisplaysleepCurrentlyDisplayed[MODE_DISPLAYSLEEP_DIGIT_POS_SIZE];  // what is currently on the screen?

// Put our resources in an easy to find list
static const PulseResource modeDisplaySleepWatchDigits[] = {
  IMAGE_MODE_DISPLAYSLEEP_0_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_1_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_2_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_3_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_4_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_5_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_6_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_7_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_8_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_9_SMALL,
  IMAGE_MODE_DISPLAYSLEEP_COLON_SMALL,
};

void mode_displaysleep_draw_digit(int iPosition, int iDigit) {
  multi_debug("digit %i %i\n", iPosition, iDigit);

  if(modeDisplaysleepCurrentlyDisplayed[iPosition] != iDigit) {
    modeDisplaysleepCurrentlyDisplayed[iPosition] = iDigit;
    pulse_draw_image(modeDisplaySleepWatchDigits[iDigit], 
      SCREEN_WIDTH - 12, 50 + (7 * iPosition));
  }
}

void mode_displaysleep_tick_tock() {
  multi_debug("mode_displaysleep_tick_tock\n");

  pulse_cancel_timer(&modeDisplaysleepingTimerID); 
  assert(modeDisplaysleepingTimerID == -1);

  modeDisplaysleepingTimerID = pulse_register_timer(10000, // slow
                   &mode_displaysleep_tick_tock, 0); // pulse
  assert(modeDisplaysleepingTimerID != -1);

  mode_displaysleep_draw_digit(0, multiTimeNow.tm_hour / 10); // Digit *?:??
  mode_displaysleep_draw_digit(1, multiTimeNow.tm_hour % 10); // Digit ?*:??
  // position 2 has the colon!
  mode_displaysleep_draw_digit(3, multiTimeNow.tm_min / 10);  // Digit ??:*?
  mode_displaysleep_draw_digit(4, multiTimeNow.tm_min % 10);  // Digit ??:?*
}

// It's time to sleep!
void mode_displaysleep_power_down() {
  multi_debug("mode_displaysleep_power_down - time to sleep\n");
  // Immediately stop the watch doing anything else
  multi_external_sleep_init();

  pulse_cancel_timer(&modeDisplaysleepPowerDownTimerID); // pulse

  // Clear the display
  pulse_blank_canvas();

  pulse_oled_set_brightness(0); // dark

  // Clear our positions
  for (int i=0; i<MODE_DISPLAYSLEEP_DIGIT_POS_SIZE; i++) {
    modeDisplaysleepCurrentlyDisplayed[i] = MODE_DISPLAYSLEEP_NO_DIGIT;
  }

  // Display the colon
  mode_displaysleep_draw_digit(2, 10); // Digit ??*?? <-- colon

  // Now start the proper loop
  mode_displaysleep_tick_tock(); 
}

// This is called regularly to stop the watch "going to sleep"; all we
// want to do here is cancel our existing timer and create a new one
void mode_displaysleep_update_power_down(int iPowerDownMS) {
  multi_debug("mode_displaysleep_update_power_down %i\n", iPowerDownMS);

  // Ensure our own personal "active" loop is cancelled
  pulse_cancel_timer(&modeDisplaysleepingTimerID); 
  assert(modeDisplaysleepingTimerID == -1);

  pulse_cancel_timer(&modeDisplaysleepPowerDownTimerID); // pulse
  assert(modeDisplaysleepPowerDownTimerID == -1);

  modeDisplaysleepPowerDownTimerID = pulse_register_timer(iPowerDownMS,
                   &mode_displaysleep_power_down, 0); // pulse
  assert(modeDisplaysleepPowerDownTimerID != -1);
}

// The main init function
// In here we need to hook in the external update function and the
// flag so that we are not treated like a normal watchface.
void mode_displaysleep_watch_functions(const enum multi_function_table iFunc,
                                       ...) {
  multi_debug("enum %i\n", iFunc);
  // Only COLDBOOT and MODEINIT will ever be called in here because we are not
  // a real watch face (as far as a user being able to select us manually).
  switch (iFunc) {
    case COLDBOOT:
      // We tell the framework to use our power down function we want to use
      multi_external_update_power_down_func =
             &mode_displaysleep_update_power_down;
      break;
    case MODEINIT:
      multiSkipThisWatchMode = true; // we are not a normal watch face
      break;
    default: // ignore features we do not use
      break;
  }
}

