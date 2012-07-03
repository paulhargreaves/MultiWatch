// Multi-watch engine
// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Enables (relatively) simple plug-n-play single-tasking multi-watch
// so that the watch will display different faces (modes)
// 
// Disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "watch_config.h" // pull in the function call structures

// Forward declarations
void multi_woke_from_sleep_by_button(void);
void multi_change_watch_mode(void);
void multi_timer_fired(void *);
void multi_received_bluetooth_data(const uint8_t *);
void multi_change_brightness(void *);
void multi_fake_function(void);
void multi_tick_tock_loop(void);
void multi_cancel_all_multi_timers(void);
void main_app_loop(void);
void multi_button_down_long(void);
void main_app_init();

typedef struct {
  int32_t callbackID;
  int32_t *userIDVariableLocation;
  PulseCallback userFunction;
  void *data;
} multiTimerCallbackStruct;

// Defines for defaults that we are not hard hardcoding
#define MULTI_CALLBACK_STORAGE_SIZE 10 // how many callbacks can we store
#define MULTI_DEFAULT_POWERDOWN_TIME (10 * 1000) // in milliseconds

#define MULTI_FADE_MINIMUM_BRIGHTNESS 0
#define MULTI_FADE_MAXIMUM_BRIGHTNESS 100
#define MULTI_FADE_BRIGHTNESS_STEP 10 
#define MULTI_FADE_ADJUST_TIME_MS 20

#define MULTI_MAX_NOTIFICATION_CALLBACKS 10

#define MULTI_NOTIFICATIONS_PAUSE_TIMEOUT 14000

int multiCurrentWatchMode = -1; // first mode is 0, -1 will be incremented 
//int multiLastWatchMode; // used to see if button_up should be ignored
int multiPowerDownTimeout; // how long do we want to powerdown for?
int multiPowerDownPausedTimeoutSave; // a copy of multiPowerDownTimeout if we are paused
bool multiPauseAllTimers = false; // if true we are not passing users timer calls back until later - used for external notification pause
bool multiPoweredDown = false; // if true then some external sleep function is active
int32_t multiPauseAllTimersTimerID = -1; // used to store the timer for pausing


// Space for our timer callbacks
multiTimerCallbackStruct 
       multiTimerCallbackStore[MULTI_CALLBACK_STORAGE_SIZE]; 

// Space for our notification callback
PulseCallback multiNotificationsCallbackFunc;

int32_t multiChangeModeTimerID = -1; // store the vlong press timeout
int32_t multiVibeOnTimerID = -1; // used to store the vibe timeout
int32_t multiTickTockTimerID = -1; // used to store the vibe timeout
int multiTimerIDVariable = 0; // used to push out higher ids to catch bugs

int32_t multiLongPressTimerID = -1; // for BUTTONDOWNLONGPRESS

// This function is called once after the watch has booted up
// and the OS has loaded
void main_app_init() {
  multi_debug("main_app_init()\n");
  // Clear our callback table
  for (unsigned int i=0; i<MULTI_CALLBACK_STORAGE_SIZE; i++) {
    multiTimerCallbackStore[i].callbackID = -1;
  }

  multiBluetoothIsConnected = false; // probably not connected immediately
  multi_external_update_power_down_func = NULL; // no function

  // Call all the modes boot init functions
  int modes = 0;
  while ( multi_watch_functions[modes] != NULL) {
    multi_watch_functions[modes](COLDBOOT);
    modes++;
  }

  // Switch to the first mode
  multi_change_watch_mode();

  pulse_register_callback(ACTION_WOKE_FROM_BUTTON, 
                          (PulseCallback) &multi_woke_from_sleep_by_button);
  pulse_register_callback(ACTION_HANDLE_NON_PULSE_PROTOCOL_BLUETOOTH_DATA,
                          (PulseCallback) &multi_received_bluetooth_data);
  multi_debug("*************** main init complete\n");
}

// Called by an external sleep func
void multi_external_sleep_init(void) {
  multi_cancel_all_multi_timers();
  multiPoweredDown = true;
  multiPauseAllTimers = true;
}

// Just woken up by the button? Call the watch face... will likely redraw
void multi_woke_from_sleep_by_button() {
  multi_debug("woke_from_sleep_by_button in mode %i\n", multiCurrentWatchMode);

  // reset our timers
  multiChangeModeTimerID = -1; 
  multiVibeOnTimerID = -1;
  multiTickTockTimerID = -1;
  multiLongPressTimerID = -1; 
  assert(multiPauseAllTimers == false);

  // clear the canvas
  pulse_blank_canvas(); 

  pulse_oled_set_brightness(MULTI_FADE_MINIMUM_BRIGHTNESS);

  multi_watch_functions[multiCurrentWatchMode](BUTTONWAKE);

  // set power down timer to whatever the user set
  multi_update_power_down_timer(multiPowerDownTimeout);

  pulse_oled_set_brightness(MULTI_FADE_MAXIMUM_BRIGHTNESS); 

  // Trigger the tick tock loop for the function
  multi_tick_tock_loop();
}

// Trigger this modes tick tock function if it exists and then
// keep looping. If the mode doesn't need it then we won't keep looping.
void multi_tick_tock_loop() {
  multi_debug("multi_tick_tock_loop. timems = %i\n", multiLoopTimeMS);
  if (multiLoopTimeMS) {
    multi_debug("calling the watch mode and re-registering\n");
    multi_watch_functions[multiCurrentWatchMode](MAINLOOP); // call user func

    multi_debug("tick tock id was %i\n", multiTickTockTimerID);
    multi_register_timer(&multiTickTockTimerID, multiLoopTimeMS, 
                         (PulseCallback) &multi_tick_tock_loop, 0); 
    multi_debug("tick tock id %i\n", multiTickTockTimerID);
    assert(multiTickTockTimerID != -1);
  }
}

void main_app_handle_button_down() {
  multi_debug("main_app_handle_button_down[%i]\n", multiCurrentWatchMode);
  if (multiPauseAllTimers) {
    multi_debug("ignoring button down as we are paused\n");
    return;
  }

  // Set up timer to change mode
  multi_register_timer(&multiChangeModeTimerID, multiModeChangePressTime,
                      (PulseCallback) &multi_change_watch_mode, 0); 

  // Set up timer to send long presses
  multi_register_timer(&multiLongPressTimerID, multiButtonDownLongPressTimeMS,
                      (PulseCallback) &multi_button_down_long, 0);

  // reset power down timer to whatever the user set
  multi_update_power_down_timer(multiPowerDownTimeout);
  
  // Tell the user function what just happened
  multi_watch_functions[multiCurrentWatchMode](BUTTONDOWN);
}

// The button was pressed for a while? Send the message
void multi_button_down_long(void) {
  multi_debug("multi_button_down_long\n");
  multi_cancel_timer(&multiLongPressTimerID);
  assert(multiLongPressTimerID == -1);

  // Tell the user function what just happened
  multi_watch_functions[multiCurrentWatchMode](BUTTONDOWNLONGPRESS);
}

// Starts the motor up for a given amount of time
void multi_vibe_for_ms(uint32_t iTimeToVibeForInMS) {
  multi_debug("multi_vibe_for_ms %i\n", iTimeToVibeForInMS);

  // Setup a timer to turn the vibe back off. 
  multi_register_timer(&multiVibeOnTimerID, iTimeToVibeForInMS, 
                       (PulseCallback) &pulse_vibe_off, 0); 
                       //(PulseCallback) &multi_vibe_off, 0); 
  assert(multiVibeOnTimerID != -1);

  // Gentlemen, start your motors!
  pulse_vibe_on();
}
  
void multi_notification_handler_pause_finished() {
  multi_debug("multi_notification_handler_pause_finished\n");

  pulse_cancel_timer(&multiPauseAllTimersTimerID); // pulse
  assert(multiPauseAllTimersTimerID == -1);
  // Keep powered up, also set the time back to what it was before the event
  multi_update_power_down_timer(multiPowerDownPausedTimeoutSave);

  multiPauseAllTimers = false;// Release existing timers

  // Tell the face it was overwritten
  multi_watch_functions[multiCurrentWatchMode](SCREENOVERWRITTEN);

  // If it cannot handle being overwritten then we just refresh the screen
  if ( !multiMyWatchFaceCanHandleScreenOverwrites ) {
    multi_debug("watch face cannot handle pause so resetting it\n");
    multiCurrentWatchMode--; // change_watch_mode increments back again
    multi_change_watch_mode();
  }

}

// Externalised and used by notification apps 
// If the current face cannot handle screen overwrites then we set the mode
// back by 1 and then change the mode again - the effect is to restart the
// current face as if we had just changed to it with the button.
void multi_external_notification_handler_complete() { 
  multi_debug("multi_external_notification_handler_complete\n");
  assert(multiCurrentWatchMode != -1);
  multiPauseAllTimers = true;// Pause existing timers
  
  pulse_cancel_timer(&multiPauseAllTimersTimerID); // pulse
  assert(multiPauseAllTimersTimerID == -1);
  multiPauseAllTimersTimerID = pulse_register_timer(
    MULTI_NOTIFICATIONS_PAUSE_TIMEOUT, // pulse
    (PulseCallback) &multi_notification_handler_pause_finished, 0);
  assert(multiPauseAllTimersTimerID != -1);
  multi_debug("delay created id %i\n", multiPauseAllTimersTimerID);
}
  
// Change to the next watch mode
void multi_change_watch_mode() {
  multi_debug("multi-change-watch-mode from %i\n", multiCurrentWatchMode);
  multi_debug("change-current timer says %i\n", multiChangeModeTimerID);

  // First we cancel all the outstanding timers
  multi_cancel_all_multi_timers();

  // Turn off the vibe motor if running
  //multi_vibe_off();
  pulse_vibe_off();

/*  // Remember what the last mode was, for button_up so it can ignore first press
  if (!multiPauseAllTimers) { // but not if we were just pausing...
    multiLastWatchMode = multiCurrentWatchMode;
  }
*/

  // Set powerdown timer now..., the individual modes may override
  multi_update_power_down_timer(MULTI_DEFAULT_POWERDOWN_TIME);

  // Reset options that the face may need to overwrite
  multiLoopTimeMS = 200; // how long we normally loop a watch for, 200ms
  multiModeChangePressTime = MULTI_MODE_CHANGE_PRESS_TIME_DEFAULT;
  multiMyWatchFaceCanHandleScreenOverwrites = false;
  multiButtonDownLongPressTimeMS = 50000; // long time... 

  // Update the current time
  //pulse_get_time_date(&multiTimeNow);
  main_app_loop(); // using main_app_loop so that utility apps can adjust "time"

  // Now we change the mode
  multiSkipThisWatchMode = true; 
  while (multiSkipThisWatchMode) {
    multiSkipThisWatchMode = false; // do not skip unless the mode wants it

    multiCurrentWatchMode++;
    multi_debug("Trying watch mode %i\n", multiCurrentWatchMode);
    // NULL? Wrap back to first face
    if (multi_watch_functions[multiCurrentWatchMode] == NULL ) {
      multi_debug("Looping back to first watch mode\n");
      multiCurrentWatchMode = 0;
    }

    // Blank the current name of the watch face
    multiMyWatchFaceName = NULL;

    // Then we tell the mode it needs to init variables etc
    // In here the mode may choose to set multiSkipThisWatchMode
    multi_watch_functions[multiCurrentWatchMode](MODEINIT);

    multi_debug("The new watch face is called %s\n", multiMyWatchFaceName);
  }

  // Then we pretend we just work up
  multi_woke_from_sleep_by_button();

  multi_debug("change complete\n");
}


void multi_update_power_down_timer(uint32_t iScheduleSleepInMS) {
  multi_debug("multi_update_power_down_timer %i\n", iScheduleSleepInMS);

  multiPowerDownTimeout = iScheduleSleepInMS;
  if (!multi_external_update_power_down_func) {
    pulse_update_power_down_timer(multiPowerDownTimeout); // normal power timer
  } else {
    multi_external_update_power_down_func(multiPowerDownTimeout); // users func
  }
}

// Go to sleep almost immediately
void multi_please_sleep_now(int iHowQuickly) {
  int multiRealSleepTimeout;
  multiRealSleepTimeout = multiPowerDownTimeout; 
  multi_update_power_down_timer(iHowQuickly); 
  multiPowerDownTimeout = multiRealSleepTimeout; // set the proper value back
}

// Register a timer. Stores the return value so we can cancel it when the
// watch mode changes. Unlike pulse_register_timer this directly changes the
// variable that the user passes in (like pulse_cancel_timer); NULL is not
// valid.
// DANGER: DANGER: When you use this function if you pass a variable location
// that then goes away (e.g. declare a variable in a function, register the
// timer, then return) then we will start randomly overwriting random locations
// of memory. This is exactly the same as pulse_cancel_timer would have done
// and generally you should use global variables for your timers rather than
// local variables.
// If you call this with timer already set then we will first cancel the
// existing timer... so try to avoid re-using variables for multiple timers
void multi_register_timer(int32_t *iUserLocationForID, uint32_t iTimeoutMS, 
                          PulseCallback iCallback, void *iData) {
  multi_debug("multi_register_timer %i for %i ms\n", iUserLocationForID, 
              iTimeoutMS);

  assert(iUserLocationForID); // no NULL passed in

  // Cancel the existing timer if it was running
  multi_cancel_timer(iUserLocationForID); 

  // Find an empty slot
  int emptySlot = -1; // will contain the slot number
  for (int i = 0; i<MULTI_CALLBACK_STORAGE_SIZE && emptySlot == -1; i++) {
     if ( multiTimerCallbackStore[i].callbackID == -1 ) {
       emptySlot = i;
       multiTimerCallbackStore[emptySlot].userFunction = iCallback;
       multiTimerCallbackStore[emptySlot].data = iData;
       multiTimerCallbackStore[emptySlot].userIDVariableLocation = 
                                iUserLocationForID;
       *iUserLocationForID = emptySlot; // tell the caller the slot number
       while (multiTimerCallbackStore[emptySlot].callbackID == -1) {
         multiTimerCallbackStore[emptySlot].callbackID =
               pulse_register_timer(iTimeoutMS,   // pulse!
               (PulseCallback) &multi_timer_fired, (void*)emptySlot);
       }
     }
     multi_debug("call structure %i=%i\n", i,
                 multiTimerCallbackStore[i].callbackID);
  }
  multi_debug("callback watermark is at %i of %i\n", emptySlot, 
             MULTI_CALLBACK_STORAGE_SIZE-1);
  assert(emptySlot != -1);
}

void multi_timer_fired(void *iData) {
  int id = (int)iData;
  multi_debug("multi_timer_fired %i\n", id);

  // Are we pausing the users timers? If so, just set it up again and
  // return. This must be kept in-step with multi_register_timer
  if (multiPauseAllTimers) {
    multi_debug("pause requested for this id. Recreating.\n");
    pulse_cancel_timer(&multiTimerCallbackStore[id].callbackID); // pulse!
    multiTimerCallbackStore[id].callbackID =
               pulse_register_timer(500,   // pulse!  And we're waiting 500ms
               (PulseCallback) &multi_timer_fired, (void*)id);
    // we do not touch the rest of the users structure so should be able to
    // call back without any issues
    return;
  }

  // Cleanly cancel the users timer - note we still use the structure
  // immediately afterwards. This is (relatively) safe because the
  // structure is not completely wiped in cancel_timer and we use it before
  // any other calls are made so it will not have been touched. 
  multi_cancel_timer(multiTimerCallbackStore[id].userIDVariableLocation); 
  // Fire the users function with the data value
  multiTimerCallbackStore[id].userFunction(multiTimerCallbackStore[id].data);

  // At this point the structure is in an unknown state as the call we just
  // made could have reused it... so do not use anymore
}

void multi_cancel_timer(int32_t *iTimerptr) {
  multi_debug("multi_cancel_timer %i\n", iTimerptr);
  
  if((int)iTimerptr == -1) { return; };  // no need to cancel
  assert(iTimerptr != 0);

  int id=*iTimerptr;

  multi_debug("id is %i = %i\n", iTimerptr, id);
  // No timer specified? Only should happen at first boot...
  if(id == -1) { return; }; 

  multi_debug("cancelling timer for %i user id was %i\n", 
              iTimerptr, id);
  assert(id>=0 && id <MULTI_CALLBACK_STORAGE_SIZE);
  assert(multiTimerCallbackStore[id].callbackID != -1);

  pulse_cancel_timer(&multiTimerCallbackStore[id].callbackID);
  assert(multiTimerCallbackStore[id].callbackID == -1);
  *iTimerptr = multiTimerCallbackStore[id].callbackID; // set the user var
  // Do not clear anything else in the structure because it may be immediately
  // used if it's part of the multi_timer_fired. 
  // It can, and will, be cleared/reused on the next call to
  // multi_register_timer so no other function should consider the data safe.
  multi_debug("cancel complete\n");
}
  

void main_app_handle_button_up() {
  multi_debug("multi_app_handle_button_up[%i]\n", multiCurrentWatchMode);

  // time to wake up from an external sleep function?
  if (multiPoweredDown) {
    multiPoweredDown = false; // ok.. we're back
    multiPauseAllTimers = false; // allow other timers to work again
    multi_woke_from_sleep_by_button(); // pretend we just woke up
    return;
  }
  
  // Is someone cancelling an external alert? If so, allow it to be cancelled
  if (multiPauseAllTimers) {
    multi_debug("ignoring button up but cancelling alert pause\n");
    multi_notification_handler_pause_finished(); // 
    return;
  }

  // Cancel pending mode change
  multi_debug("multiChangeModeTimerID=%i\n", multiChangeModeTimerID);
  multi_cancel_timer(&multiChangeModeTimerID); // cancel pending mode change
  assert(multiChangeModeTimerID == -1);

  // If there isn't a button_down timer (long press) running then
  // this must be the button release so ignore it
  if ( multiLongPressTimerID == -1 ) {
    multi_debug("ignoring button up as no long button timer is running.\n");
    return;
  }

  multi_cancel_timer(&multiLongPressTimerID); // cancel button long press
  assert(multiLongPressTimerID == -1);

/* -- Can this go now?
  // Caused by a mode change? We are now in the new mode so ignore it
  if ( multiCurrentWatchMode != multiLastWatchMode &&
       multiLastWatchMode != -1 ) {  // -1 is no mode previously
    multiLastWatchMode = multiCurrentWatchMode;
    return;
  }
*/

  // Tell the user functions button was released
  multi_watch_functions[multiCurrentWatchMode](BUTTONUP);
}

// Main loop. This function is called frequently.
// No blocking calls are allowed in this function or else the watch will reset.
// The inPulse watchdog timer will kick in after 5 seconds if a blocking
// call is made.
void main_app_loop() {
  // get the current time and date
  //multi_debug("get date\n");
  pulse_get_time_date(&multiTimeNow);

  // If the user has created their own main loop then call it
  int modes = 0;
  while ( multi_watch_functions[modes] != NULL) {
    multi_watch_functions[modes](APPLOOP);
    modes++;
  }
}


void multi_cancel_all_multi_timers() {

  for (unsigned int id=0; id<MULTI_CALLBACK_STORAGE_SIZE; id++) {
    if (multiTimerCallbackStore[id].userIDVariableLocation) {
      multi_cancel_timer(multiTimerCallbackStore[id].userIDVariableLocation); 
    }
  }
}

// This function is called whenever the processor is about to sleep
// (to conserve power). The sleep functionality is scheduled with
// pulse_update_power_down_timer(uint32_t)
void main_app_handle_doz() { 
  // Cancel all the users timers (and most of ours)...
  multi_cancel_all_multi_timers();

  // hack the brightness down
  for(int i=MULTI_FADE_MAXIMUM_BRIGHTNESS;i>=MULTI_FADE_MINIMUM_BRIGHTNESS;
      i=i-MULTI_FADE_BRIGHTNESS_STEP) {
    if (i >= MULTI_FADE_MINIMUM_BRIGHTNESS) {
      multi_debug("Bright %i\n", i);
      pulse_oled_set_brightness(i);
      pulse_mdelay(MULTI_FADE_ADJUST_TIME_MS); 
    }
  }
}

void main_app_handle_hardware_update(enum PulseHardwareEvent iEvent) {
  switch(iEvent) {
    case BLUETOOTH_CONNECTED:
      multiBluetoothIsConnected = true; break;
    case BLUETOOTH_DISCONNECTED:
      multiBluetoothIsConnected = false; break;
    case BATTERY_CHARGING:
      multiBatteryCharging = true; break;
    case BATTERY_NOT_CHARGING:
      multiBatteryCharging = false; break;
    default: break; // make compiler happy
  }
  // Now tell all the watch faces, regardless of their state
  int modes = 0;
  while ( multi_watch_functions[modes] != NULL) {
    multi_watch_functions[modes](HARDWARECHANGE);
    modes++;
  }
}

// Handles bluetooth messages
void multi_received_bluetooth_data(const uint8_t *iBuffer) {
  multiBluetoothRecBuffer = (void*)iBuffer;
  multi_watch_functions[multiCurrentWatchMode](BLUETOOTHREC);
}


// Until one goes into pulse_*, may as well have a common one apps can use
void multi_draw_box(int x, int y, int width, int height, color24_t colour) {
  // Work out the x2 co-ordinate, stopping at the edge of the screen
  int x2 = x + width;
  if (x2 >= SCREEN_WIDTH) {
    x2 = SCREEN_WIDTH - 1;
  } 

  // Work out the y2 co-ordinate, stopping at the bottom of the screen
  int y2 = y + height;
  if (y2 >= SCREEN_HEIGHT) {
    y2 = SCREEN_HEIGHT - 1;
  }

  // Bail if the positions are off screen
  if (y < 0 || y >= SCREEN_HEIGHT || x < 0 ||
      x >= SCREEN_WIDTH) {
    multi_debug("Not drawing (%i, %i)\n", x, y);
    return;
  }

  assert(y < SCREEN_HEIGHT);
  assert(y >= 0);
  assert(y2 < SCREEN_HEIGHT);
  assert(y2 >= 0);
  assert(x < SCREEN_WIDTH);
  assert(x2 < SCREEN_WIDTH);
  assert(x >= 0);
  assert(x2 >= 0);
  assert(height >= 0);
  assert(width >= 0);
  assert(x2 >= x);
  assert(y2 >= y);
  //multi_debug("draw (%i, %i) to (%i, %i) color %i\n", x, y, x2, y2, colour);

  // Set the window and draw the pixels
  pulse_set_draw_window(x,y,x2,y2);

  for(int i=0; i<(width+1)*(height+1); i++) {
    pulse_draw_point24(colour);
  }
}

// xor128 function from http://en.wikipedia.org/wiki/Xorshift
uint32_t multi_rand(void) {
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123;
  uint32_t t;

  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

// Register
void multi_register_notifications(PulseCallback functionToCall) {
  assert(multiNotificationsCallbackFunc == NULL);
  multiNotificationsCallbackFunc = functionToCall;
  assert(multiNotificationsCallbackFunc != NULL);

  pulse_register_callback(ACTION_NEW_PULSE_PROTOCOL_NOTIFICATION,
                 (PulseCallback) &multi_notifications_new_notification);
}

// Received a notification; call the user func
void multi_notifications_new_notification(PulseNotificationId id) {
  assert(multiNotificationsCallbackFunc != NULL);
  multi_debug("Notification received. Calling user func\n");
  #ifdef DEBUG
  struct PulseNotification *notification = pulse_get_notification(id);
  multi_debug("Type: %i\n", notification->type);
  multi_debug("Sender: %s\n", notification->sender);
  multi_debug("Body: %s\n", notification->body);
  #endif

  pulse_blank_canvas();
  pulse_oled_set_brightness(100);

  // Try to prevent the watch from turning off...
  multiPowerDownPausedTimeoutSave = multiPowerDownTimeout;
  multi_update_power_down_timer(MULTI_NOTIFICATIONS_PAUSE_TIMEOUT+1000); // keep powered up

  multiNotificationsCallbackFunc((void *)id);
}

uint32_t multi_time_in_sec(void) { 
  return multiTimeNow.tm_hour * 60 * 60 + multiTimeNow.tm_min * 60 + 
         multiTimeNow.tm_sec; 
}

// END
