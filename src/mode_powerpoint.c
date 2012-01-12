// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Powerpoint
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Enables (relatively) simple plug-n-play single-tasking multi-watch
// so that the watch will display different faces (modes)
// 
// Disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "mode_powerpoint.h"

bool modePowerpointJustChangedWatchMode; // suppress message on changeover?
PulseResource modePowerpointImage; // Powerpoint image to show
int modePowerpointBTCounter; // The next counter we will send to the computer
int modePowerpointBTExpectValue; // response we expect back - bt_counter
int32_t modePowerpointBTWaitTimerID; // timer to wait for bt response

#define MODE_POWERPOINT_BT_COUNTER_START 150 // must be >128 <255
#define MODE_POWERPOINT_BT_COUNTER_MAX 230 // must be >BT_COUNTER_START <255

void mode_powerpoint_watch_functions(const enum multi_function_table iFunc, ...) {
  multi_debug("enum %i\n", iFunc);
  va_list varargs;
  va_start(varargs, iFunc);
  switch (iFunc) {
    case MODEINIT:
      mode_powerpoint_init();
      break;
    case BUTTONWAKE:
      mode_powerpoint_woken_by_button();
      break;
    case MAINLOOP:
      mode_powerpoint_draw_watch_face();
      break;
    case BUTTONUP:
      mode_powerpoint_button_pressed();
      break;
    case BLUETOOTHREC:
      mode_powerpoint_got_bluetooth_data(va_arg(varargs, uint8_t *));
      ///hmm..
      break;
    default: // ignore features we do not use
      break;
  }
  va_end(varargs);
}

void mode_powerpoint_woken_by_button() {
  // Just woken up - display is blank
  modePowerpointBTWaitTimerID = -1; // timer to wait for bt response

  printf("   Time %02i:%02i\n", multiTimeNow.tm_hour, multiTimeNow.tm_min);

  // Have we just changed watch mode? If not, assume we are changing slide
  if ( modePowerpointJustChangedWatchMode == false ) {
    mode_powerpoint_button_pressed();
  }
  modePowerpointJustChangedWatchMode = false;

}


void mode_powerpoint_init() {
  multi_debug("mode_powerpoint_init\n");

  // Reset all of our globals
  modePowerpointJustChangedWatchMode = true;
  modePowerpointBTCounter = MODE_POWERPOINT_BT_COUNTER_START;
}

void mode_powerpoint_send_vibe_failed() {
  for(int i=0; i<2; i++) {
    // Warn the user we failed...
    pulse_vibe_on();
    pulse_mdelay(MODE_POWERPOINT_VIBE_TIME * 2);
    pulse_vibe_off();
    pulse_mdelay(MODE_POWERPOINT_VIBE_TIME * 3);
  }
  modePowerpointBTCounter++;
}

void mode_powerpoint_button_pressed() {
  multi_debug("powerpoint button pressed");

  // Increment our bluetooth counter
  modePowerpointBTCounter++;
  if (modePowerpointBTCounter > MODE_POWERPOINT_BT_COUNTER_MAX) {
    modePowerpointBTCounter = MODE_POWERPOINT_BT_COUNTER_START;
  }

  if (multiBluetoothIsConnected) {
    modePowerpointBTExpectValue = modePowerpointBTCounter;
    multi_register_timer(&modePowerpointBTWaitTimerID, 
         MODE_POWERPOINT_BT_WAIT_TIMEOUT,
         (PulseCallback) &mode_powerpoint_bt_failed, 0); 
    assert(modePowerpointBTWaitTimerID != -1);
    
    assert(modePowerpointBTCounter >= MODE_POWERPOINT_BT_COUNTER_START);
    pulse_send_bluetooth_int(modePowerpointBTCounter); // send hello
  } else { // multi_is_bluetooth_connect() == false
    mode_powerpoint_send_vibe_failed();
  }
}

void mode_powerpoint_bt_failed(void) {
  multi_debug("mode_powerpoint_bt_failed waiting for %i\n",
              modePowerpointBTExpectValue);
  modePowerpointBTCounter++; // we don't care if it overruns here...
  mode_powerpoint_send_vibe_failed();
}

void mode_powerpoint_draw_watch_face() {
  multi_debug("mode_powerpoint_draw_watch_face\n");

  modePowerpointImage = IMAGE_MODE_POWERPOINT;
  // Problems connecting? Put a big X
  if ( multiBluetoothIsConnected == false ) {
    modePowerpointImage = IMAGE_MODE_POWERPOINT_NOT_READY;
  }

  pulse_draw_image(modePowerpointImage,
                   (SCREEN_WIDTH - MODE_POWERPOINT_IMAGE_WIDTH)/2,
                   (SCREEN_HEIGHT - MODE_POWERPOINT_IMAGE_HEIGHT)/2);
}

// got a bluetooth message... hurrah?
void mode_powerpoint_got_bluetooth_data(const uint8_t *iBuffer) {
  multi_debug("mode_powerpoint_got_bluetooth_data\n");
  modePowerpointBTCounter++; // increment, we don't need to check its valid

  // We check if this is the data we were looking for
  if (iBuffer[0] == modePowerpointBTExpectValue) {
    // It is - cancel the failure timer
    multi_cancel_timer(&modePowerpointBTWaitTimerID);
    // Send the next slide key
    pulse_send_bluetooth_int(MODE_POWERPOINT_NEXT_SLIDE); // send hello
    multi_vibe_for_ms(MODE_POWERPOINT_VIBE_TIME); // vibe away!
  }

  // or no idea what the data was, just going to ignore it completely
}
