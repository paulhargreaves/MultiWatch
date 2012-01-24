// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Hourly chime
// Buzzes the watch every hour from 8:59am until 5:59pm on workdays. 
// Pressing the button enables or disables the alarm
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_hourlychime.h"


PulseAlarm *modeHourlychimeAlarm;

void mode_hourlychime_refresh_alarm_data(void) {
  #ifndef PULSE_SIMULATOR
  modeHourlychimeAlarm = pulse_get_alarm(); 
  #endif
}

void mode_hourlychime_show_next_alarm(void) {
  mode_hourlychime_refresh_alarm_data();
  
  const char daysOfWeek[][4] = 
                           { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }; 
  printf("* Now\n\n");
  printf("Time: %02i:%02i\n", multiTimeNow.tm_hour, multiTimeNow.tm_min);
  printf("Day: [%i] %s\n", multiTimeNow.tm_wday,
            daysOfWeek[multiTimeNow.tm_wday]);
  printf("\n* Next alarm\n\n");

  printf("Enabled: %i\n\n", modeHourlychimeAlarm->enabled);
  if (modeHourlychimeAlarm->enabled) {
    printf("Time: %02i:%02i\n", modeHourlychimeAlarm->hour, 
           modeHourlychimeAlarm->min);

    printf("Days:\n");
    for(int i=0; i<8; i++) {
      if(modeHourlychimeAlarm->daysActive[i]) {
        printf("%s ", daysOfWeek[i]);
      }
    }
    printf("\n");
  } // alarm is enabled
  printf("\nBattery state: %i", multiBatteryCharging);
  // No more space on the OLED
}

void mode_hourlychime_clear_alarm(void) {
  multi_debug("mode_hourlychime_clear_alarm\n");
  PulseAlarm nextAlarm;
  nextAlarm.enabled = false;
  pulse_set_alarm(&nextAlarm);
  #ifdef PULSE_SIMULATOR
  modeHourlychimeAlarm->enabled = 0; // some fake data for testing
  #endif
}
  
void mode_hourlychime_set_next_alarm(void) {
  multi_debug("mode_hourlychime_set_next_alarm\n");
  PulseAlarm nextAlarm;
  
  nextAlarm.enabled = true;
  nextAlarm.hour = multiTimeNow.tm_hour + 1; // Buzz at HH+1:59
  if(multiTimeNow.tm_min < 58) { // Before HH:58? We want to buzz at HH:59 then
    nextAlarm.hour = multiTimeNow.tm_hour;
  }
  if(nextAlarm.hour < 8 || nextAlarm.hour > 17) {
    nextAlarm.hour = 8;
  }
  nextAlarm.min = 59;
  //nextAlarm.min = (multiTimeNow.tm_min + 1) % 59; // - for testing
  // nextAlarm.snoozeMin = 0; // undefined
  // nextAlarm.durationMS = 0; // undefined
  nextAlarm.alert.type = 1;
  //nextAlarm.alert.vibe_intensity = 0; // undefined
  nextAlarm.alert.on1 = 25;
  nextAlarm.alert.off1 = 10;
  nextAlarm.alert.on2 = 15;
  nextAlarm.alert.off2 = 5;
  nextAlarm.alert.on3 = 1;
  nextAlarm.alert.off3 = 1;
  nextAlarm.alert.on4 = 1;
  nextAlarm.daysActive[0] = false;
  nextAlarm.daysActive[6] = false;
  nextAlarm.daysActive[7] = false;
  for(int i=1; i<=5; i++) { // 0=sun, 6=sat - or does it? Who knows...
    nextAlarm.daysActive[i] = true;
  }
  //nextAlarm.msg = "Some message";
  multi_debug("alarm hour %i min %i\n",nextAlarm.hour, nextAlarm.min);
  pulse_set_alarm(&nextAlarm);
  #ifdef PULSE_SIMULATOR
  modeHourlychimeAlarm->enabled = 1; // some fake data for testing
  #endif
}

// We were just woken by the alarm...
void mode_hourlychime_woke_by_alarm(void) {
  // Refresh the time since at this point the main_app_loop hasn't fired...
  // sigh...
  pulse_get_time_date(&multiTimeNow);

  // Set the next alarm
  mode_hourlychime_set_next_alarm();

  // Go back to sleep... but need to wait for the alarm to finish since
  // the pulse alarm code doesn't prevent powering off... and mdelay would
  // be bad here as well...
  multi_please_sleep_now(500);
}

void mode_hourlychime_watch_functions(const enum multi_function_table iFunc, ...) {
  multi_debug("enum %i\n", iFunc);
  //va_list varargs;
  //va_start(varargs, iFunc);
  switch (iFunc) {
    case COLDBOOT:
      // Here is where we set up this special watch mode
      pulse_register_callback(ACTION_ALARM_FIRING, 
                              (PulseCallback) &mode_hourlychime_woke_by_alarm);
      #ifdef PULSE_SIMULATOR
      #include <stdlib.h>
      modeHourlychimeAlarm = malloc(sizeof(PulseAlarm));
      modeHourlychimeAlarm->enabled = 1; // some fake data for testing
      modeHourlychimeAlarm->hour = 10; // fake
      modeHourlychimeAlarm->min = 42; // fake
      for(int i=1; i<=5; i++) { // fake
        modeHourlychimeAlarm->daysActive[i] = true; // fake
      } 
      #endif
      break;
    case BUTTONWAKE:
      mode_hourlychime_show_next_alarm();
      break;
    case BUTTONUP:
      mode_hourlychime_refresh_alarm_data();
      if (modeHourlychimeAlarm->enabled) {
        mode_hourlychime_clear_alarm();
      } else {
         mode_hourlychime_set_next_alarm();
      }
      pulse_blank_canvas();
      mode_hourlychime_show_next_alarm();
      break;
    case MODEINIT:
      multiLoopTimeMS = 0;
      // Here is where we tell the framework we do not want to be a face
      //multiSkipThisWatchMode = true;
      break;
    default: // ignore features we do not use
      break;
  }
  //va_end(varargs);
}

