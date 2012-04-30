// (C)2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Hourly chime 2
// Buzzes the watch every hour from 8:59am until 5:59pm on workdays. 
// and every 15 minutes on the 15 minute window (adjustable in the .h)
// If compiled in, will always happen, no way to disable.
//
// It does not use the alarm function, instead is hooking into the
// main_app_loop. This has a couple of advantages... it doesn't need to worry
// about the power state, and it doesn't have to deal with the currently
// (seemingly) unreliable alarm vibe code.
// It's also much shorter code wise if the alarm code is not used elsewhere.
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_hourlychime2.h"

int modeHourlychime2OldTime = 0;
int modeHourlychime2NewTime = -1;

void mode_hourlychime2_alarm(bool longAlarm) {
  // Short alarm first, always
  pulse_vibe_on();
  pulse_mdelay(200);
  pulse_vibe_off();

  // Long alarm if we need one
  if (longAlarm) {
    pulse_mdelay(250); // pause
    pulse_vibe_on();
    pulse_mdelay(200); // longer...
    pulse_vibe_off();
  }
}

void mode_hourlychime2_alarm_check(void) {
  // Calculate the current time in a single var. We only want to do work
  // at most once per minute to save battery
  modeHourlychime2NewTime = multiTimeNow.tm_hour * 60 + multiTimeNow.tm_min;

  // Is the time different? If so lets do something with it
  if ( modeHourlychime2NewTime != modeHourlychime2OldTime ) {
    // Store the current time
    modeHourlychime2OldTime = modeHourlychime2NewTime;

    // A day we want to chime?
    if (multiTimeNow.tm_wday != 0 && multiTimeNow.tm_wday != 6) { // not sun/sat
      // Sociable hours?
      if (multiTimeNow.tm_hour >= 8 && multiTimeNow.tm_hour <= 17) {
        // Hourly chime?
        if (multiTimeNow.tm_min == 59) {
          mode_hourlychime2_alarm(true); // true == long alarm
        }
        // Quarterly chime?
        if (multiTimeNow.tm_hour > 8 && multiTimeNow.tm_min > 0) {  
          if ((multiTimeNow.tm_min / MODE_HOURLYCHIME2_SHORT_ALARM) *
                MODE_HOURLYCHIME2_SHORT_ALARM == multiTimeNow.tm_min) {
            mode_hourlychime2_alarm(false); // false == short alarm
          }
        }
      }
    }
  }

}

void mode_hourlychime2_watch_functions(const enum multi_function_table iFunc, ...) {
  multi_debug("enum %i\n", iFunc);
  //va_list varargs;
  //va_start(varargs, iFunc);
  switch (iFunc) {
    case APPLOOP:
      // Here is where we set up this special watch mode
      mode_hourlychime2_alarm_check();
      break;
    case MODEINIT:
      // Here is where we tell the framework we do not want to be a face
      multiSkipThisWatchMode = true;
      break;
    default: // ignore features we do not use
      break;
  }
  //va_end(varargs);
}

