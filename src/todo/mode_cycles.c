// Multi-watch engine
// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Cycles
// 
// Legal disclaimer: This software is provided "as-is". Use at your own peril



#include <pulse_os.h>
#include <pulse_types.h>
#include <app_resources.h>
#include <stdio.h>

#include "pulse_app.h"
#include "mode_cycles.h"


//int32_t mode_cycles_tick_tock_timer;

void mode_cycles_woken_by_button(void) {
  // Just woken up - display is blank

}


void mode_cycles_button_pressed(uint32_t downTime) {
  debug_print("cycles button pressed for %i\n",downTime);
  mode_cycles_draw_watch_face();
}


void mode_cycles_tick_tock_loop(void) {

  mode_cycles_draw_watch_face();
}


void mode_cycles_draw_watch_face() {
  struct pulse_time_tm now;
  pulse_get_time_date(&now);
  debug_print("cycles %i:%i:%i\n", now.tm_hour, now.tm_min, now.tm_sec);
  printf("cycles %i:%i:%i\n", now.tm_hour, now.tm_min, now.tm_sec);
  
}

