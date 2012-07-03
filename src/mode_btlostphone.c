// (C)2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// BT Lost Phone
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Buzzes when the phone goes out of range (is disconnected) to remind the
// wearer to take their phone with them...
//
// Special implementation notes:
// Waits for a message from hardware change and if bluetooth gets disconnected
// starts counting down a timer (30 secs or so). If BT not reconnected in that
// time it starts to vibrate until the user switches to the face and cancels it.


// Disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "mode_btlostphone.h"

void mode_btlostphone_draw_watch_face(void);
void mode_btlostphone_alarm_pulse_on(void);

#define MODE_BT_LOST_PHONE_LOST_TIMEOUT 15  // seconds
#define MODE_BT_LOST_PHONE_ALARM_SPEED 3  // seconds
bool     mode_btlostphone_alarm_ignore;
bool     mode_btlostphone_update_screen;
uint32_t mode_btlostphone_min;

// Special loop that runs even if the watch is not on
// We can't rely on pulse_register_timer etc since they aren't running if
// the screen isn't on.
// pulse_get_millis doesn't update, and the clock runs so slow that per min is
// about as good as it gets.
// Screen cannot be used.
void mode_btlostphone_apploop(void) {
  multi_debug("mode_btlostphone_apploop\n");
  // multi_debug("%i > %i\n", pulse_get_millis(), mode_btlostphone_millis);
  // ALARM if we have passed our timeout
  if (multiTimeNow.tm_min != mode_btlostphone_min) {
    multi_debug("vibe!\n");

    // schedule next "alarm"
    mode_btlostphone_min = multiTimeNow.tm_min;

    // Buzz
    pulse_vibe_on();
    pulse_mdelay(1000); // should notice this...
    pulse_vibe_off();
  }
}


void mode_btlostphone_watch_functions(const enum multi_function_table iFunc) {
  //multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case COLDBOOT:
      mode_btlostphone_alarm_ignore = false;
      break;
    // No MODEINIT since all vars are globally used when the face is not visible
    case BUTTONWAKE:
      mode_btlostphone_update_screen = true;
      break;
    case MAINLOOP:
      if (mode_btlostphone_update_screen) {
        mode_btlostphone_draw_watch_face();
        mode_btlostphone_update_screen = false;
      }
      break;
    case BUTTONUP:
      mode_btlostphone_alarm_ignore = !mode_btlostphone_alarm_ignore;
      //break; // fall through to hardware change to reset timer etc
    case HARDWARECHANGE:
      multi_debug("HARDWARECHANGE\n");
      // BUTTONUP falls through here...
      // IMPORTANT: may not be active watch face so no screen output
      mode_btlostphone_update_screen = true; // if active it will update
      // Reset our "timer"
      mode_btlostphone_min = -1;
      //break; // Falls through to APPLOOP to buzz immediately
    case APPLOOP:
      // HARDWARECHANGE falls through here
      // Hence no screen output allowed
      if (!multiBluetoothIsConnected && !mode_btlostphone_alarm_ignore) {
        mode_btlostphone_apploop();
      }
      break;
    default: // ignore features we do not use
      break;
  }
}

void mode_btlostphone_draw_watch_face() {
  pulse_blank_canvas();

//  printf("Time %02i:%02i\n\n", multiTimeNow.tm_hour, multiTimeNow.tm_min);

  printf("=BT LOST PHONE=\n\n");
  if (multiBluetoothIsConnected) {
    printf("BT connected\n");
  } else {
    printf("BT not connected");
  }

  if (mode_btlostphone_alarm_ignore) {
    printf("Alarm: disabled");
  } else { // !alarm_ignore && !alarm_triggered
    printf("Alarm: armed");
  }
}
