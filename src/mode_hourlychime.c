// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Hourly charm
// An example of hooking into the framework but not really using any of it.
// Buzzes the watch every hour from 8:59am until 5:59pm on workdays. 
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_hourlychime.h"

void mode_hourlychime_set_next_alarm(void) {
  PulseAlarm nextAlarm;
  struct pulse_time_tm now;
  pulse_get_time_date(&now);
  
  nextAlarm.enabled = true;
  nextAlarm.hour = now.tm_hour + 1; // Buzz at HH+1:59
  if(now.tm_min < 58) { // Before HH:58? We want to buzz at HH:59 then
    nextAlarm.hour = now.tm_hour;
  }
  if(nextAlarm.hour < 8 || nextAlarm.hour > 17) {
    nextAlarm.hour = 8;
  }
  nextAlarm.min = 59;
  // nextAlarm.snoozeMin = 0; // undefined
  // nextAlarm.durationMS = 0; // undefined
  nextAlarm.alert.type = 1;
  //nextAlarm.alert.vibe_intensity = 0; // undefined
  nextAlarm.alert.on1 = 35;
  nextAlarm.alert.off1 = 0;
  nextAlarm.alert.on2 = 1;
  nextAlarm.alert.off2 = 0;
  nextAlarm.alert.on3 = 1;
  nextAlarm.alert.off3 = 0;
  nextAlarm.alert.on4 = 1;
  for(int i=1; i<=5; i++) { // 0=sun, 6=sat
    nextAlarm.daysActive[i] = true;
  }
  //nextAlarm.msg = "Some message";
  multi_debug("alarm hour %i min %i\n",nextAlarm.hour, nextAlarm.min);
  pulse_set_alarm(&nextAlarm);
}

void mode_hourlychime_watch_functions(const enum multi_function_table iFunc, ...) {
  multi_debug("enum %i\n", iFunc);
  //va_list varargs;
  //va_start(varargs, iFunc);
  switch (iFunc) {
    case COLDBOOT:
      // Here is where we set up this special watch mode
      pulse_register_callback(ACTION_WOKE_FROM_BUTTON, 
                              (PulseCallback) &mode_hourlychime_set_next_alarm);
      mode_hourlychime_set_next_alarm();
      break;
    case MODEINIT:
      // Here is where we tell the framework we do not want to be a face
      multiSkipThisWatchMode = true;
    default: // ignore features we do not use
      break;
  }
  //va_end(varargs);
}

