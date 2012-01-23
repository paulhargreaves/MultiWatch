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
void multi_vibe_off(void);
void multi_change_brightness(void *);
void multi_fake_function(void);
void multi_tick_tock_loop(void);
void multi_cancel_all_multi_timers(void);

typedef struct {
  int32_t callbackID;
  int32_t *userIDVariableLocation;
  PulseCallback userFunction;
  void *data;
} multiTimerCallbackStruct;

// Defines for defaults that we are not hard hardcoding
#define MULTI_CALLBACK_STORAGE_SIZE 10 // how many callbacks can we store
#define MULTI_DEFAULT_POWERDOWN_TIME (10 * 1000) // in milliseconds
#define MULTI_MODE_CHANGE_PRESS_TIME (2 * 1000) // in milliseconds

#define MULTI_FADE_MINIMUM_BRIGHTNESS 0
#define MULTI_FADE_MAXIMUM_BRIGHTNESS 100
#define MULTI_FADE_BRIGHTNESS_STEP 10 
#define MULTI_FADE_ADJUST_TIME_MS 20


int multiCurrentWatchMode = -1; // first mode is 0, -1 will be incremented 
int multiLastWatchMode; // used to see if button_up should be ignored
int multiPowerDownTimeout; // how long do we want to powerdown for?
bool multiPauseAllTimers = false; // if true we are not passing users timer calls back until later
bool multiPoweredDown = false; // if true then some external sleep function is active
int32_t multiPauseAllTimersTimerID = -1; // used to store the timer for pausing

// Space for our timer callbacks
multiTimerCallbackStruct 
       multiTimerCallbackStore[MULTI_CALLBACK_STORAGE_SIZE]; 

uint32_t multiMSAtButtonDown; // how long was button down for?
int32_t multiButtonLongTimerID = -1; // used to store the long timeout
int32_t multiVibeOnTimerID = -1; // used to store the vibe timeout
int32_t multiTickTockTimerID = -1; // used to store the vibe timeout
int multiTimerIDVariable = 0; // used to push out higher ids to catch bugs

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
  for (unsigned int i=0; i<WATCH_MODES; i++) {
    multi_watch_functions[i](COLDBOOT);
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
  multiButtonLongTimerID = -1; 
  multiVibeOnTimerID = -1;
  multiTickTockTimerID = -1;
  assert(multiPauseAllTimers == false);

  // clear the canvas
  pulse_blank_canvas(); 

  pulse_oled_set_brightness(MULTI_FADE_MINIMUM_BRIGHTNESS);

  multi_watch_functions[multiCurrentWatchMode](BUTTONWAKE);

  // set power down timer to whatever the user set
  multi_update_power_down_timer(multiPowerDownTimeout);

  // disabled "nice" fading to recover bytes - so now we start dark and go
  // bright after the watch face has had it's buttonwake called
  /*
  // Turn the brightness up
  for(int i=MULTI_FADE_MINIMUM_BRIGHTNESS;i<=multiFadeMaxBrightness;
      i=i+MULTI_FADE_BRIGHTNESS_STEP) {
    if (i <= multiFadeMaxBrightness) {
      multi_debug("Bright %i delay %i\n", i, MULTI_FADE_ADJUST_TIME_MS);
      #ifndef PULSE_SIMULATOR
      pulse_oled_set_brightness(i);
      pulse_mdelay(MULTI_FADE_ADJUST_TIME_MS); // faster...
      #endif
    }
  }
  */
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
    multi_watch_functions[multiCurrentWatchMode](MAINLOOP);

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

  multiMSAtButtonDown = pulse_get_millis(); // get the time in ms
  multi_register_timer(&multiButtonLongTimerID, multiModeChangePressTime,
                      (PulseCallback) &multi_change_watch_mode, 0); 

  // reset power down timer to whatever the user set
  multi_update_power_down_timer(multiPowerDownTimeout);
  
  // Tell the user function what just happened
  multi_watch_functions[multiCurrentWatchMode](BUTTONDOWN);
}


// Starts the motor up for a given amount of time
void multi_vibe_for_ms(uint32_t iTimeToVibeForInMS) {
  multi_debug("multi_vibe_for_ms %i\n", iTimeToVibeForInMS);

  /*
  // Cancel any vibe timer that may be running, leaving the motor on...
  multi_cancel_timer(&multiVibeOnTimerID);
  assert(multiVibeOnTimerID == -1);

  // short timer? just do it
  if (iTimeToVibeForInMS <= 50) {
    pulse_vibe_on();
    pulse_mdelay(iTimeToVibeForInMS);
    pulse_vibe_off();
    return;
  }
  */

  // Setup a timer to turn the vibe back off. 
  multi_register_timer(&multiVibeOnTimerID, iTimeToVibeForInMS, 
                       (PulseCallback) &multi_vibe_off, 0); 
  assert(multiVibeOnTimerID != -1);

  // Gentlemen, start your motors!
  pulse_vibe_on();
}
  
// Turns off the motor, called only via multi_vibe_for_ms
void multi_vibe_off() {
  multi_debug("multi_vibe_off()\n");

  pulse_vibe_off();

  //multi_cancel_timer(&multiVibeOnTimerID); // Cancel it
  assert(multiVibeOnTimerID == -1); // Timer should be off now
}


void multi_notification_handler_pause_finished() {
  multi_debug("multi_notification_handler_pause_finished\n");

  pulse_cancel_timer(&multiPauseAllTimersTimerID); // pulse
  assert(multiPauseAllTimersTimerID == -1);
  multi_update_power_down_timer(multiPowerDownTimeout); // keep powered up!

  if ( !multiMyWatchFaceCanHandleScreenOverwrites ) {
    multi_debug("watch face cannot handle pause so resetting it\n");
    multiCurrentWatchMode--; // change_watch_mode increments back again
    multi_change_watch_mode();
  }

  multiPauseAllTimers = false;// Release existing timers
}

// Externalised and used by notification apps 
// If the current face cannot handle screen overwrites then we set the mode
// back by 1 and then change the mode again - the effect is to restart the
// current face as if we had just changed to it with the button.
void multi_external_notification_handler_complete() { 
  multi_debug("multi_external_notification_handler_complete\n");
  multiYourWatchFaceWasOverwritten = true;
  assert(multiCurrentWatchMode != -1);
  multiPauseAllTimers = true;// Pause existing timers
  
  // Create a delay
  pulse_cancel_timer(&multiPauseAllTimersTimerID); // pulse
  assert(multiPauseAllTimersTimerID == -1);
  multiPauseAllTimersTimerID = pulse_register_timer(7000, // pulse
    (PulseCallback) &multi_notification_handler_pause_finished, 0);
  assert(multiPauseAllTimersTimerID != -1);
  multi_debug("delay created id %i\n", multiPauseAllTimersTimerID);
  multi_update_power_down_timer(multiPowerDownTimeout); // keep powered up!
}
  
// Change to the next watch mode
void multi_change_watch_mode() {
  multi_debug("multi-change-watch-mode from %i\n", multiCurrentWatchMode);
  multi_debug("change-current timer says %i\n", multiButtonLongTimerID);

  // First we cancel all the outstanding timers
  multi_cancel_all_multi_timers();

  // Turn off the vibe motor if running
  multi_vibe_off();

  // Remember what the last mode was, for button_up so it can ignore first press
  if (!multiPauseAllTimers) { // but not if we were just pausing...
    multiLastWatchMode = multiCurrentWatchMode;
  }

  // Set powerdown timer now..., the individual modes may override
  multi_update_power_down_timer(MULTI_DEFAULT_POWERDOWN_TIME);

  multiLoopTimeMS = 200; // how long we normally loop a watch for, 200ms
  multiModeChangePressTime = MULTI_MODE_CHANGE_PRESS_TIME;
  multiYourWatchFaceWasOverwritten = false;
  multiMyWatchFaceCanHandleScreenOverwrites = false;

  // Update the current time
  pulse_get_time_date(&multiTimeNow);

  // Now we change the mode
  multiSkipThisWatchMode = true; 
  while (multiSkipThisWatchMode) {
    multiSkipThisWatchMode = false; // do not skip unless the mode wants it

    multiCurrentWatchMode++;
    if (multiCurrentWatchMode >= WATCH_MODES ) {
      multiCurrentWatchMode = 0;
    }

    // Then we tell the mode it needs to init variables etc
    // In here the mode may choose to set multiSkipThisWatchMode
    multi_watch_functions[multiCurrentWatchMode](MODEINIT);
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
  // immediately afterwards. This is (relatatively) safe because the
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
  
  if((int)iTimerptr == -1) { return; }; 
  assert(iTimerptr != 0);

  int id=*iTimerptr;

  multi_debug("id is %i = %i\n", iTimerptr, id);
  // No timer specified? Only should happen at first boot...
  if(id == -1) { return; }; 

  /*
  int id=userId / 100; // 100 is magic - see multi_register_timer
  if (multiTimerCallbackStore[id].callbackID == -1) {
    *iTimerptr = -1; // set the user var
    return;
  }
  */
  multi_debug("cancelling timer for %i user id was %i\n", 
              iTimerptr, id);
  assert(id>=0 && id <MULTI_CALLBACK_STORAGE_SIZE);
  assert(multiTimerCallbackStore[id].callbackID != -1);

  pulse_cancel_timer(&multiTimerCallbackStore[id].callbackID);
  assert(multiTimerCallbackStore[id].callbackID == -1);
  *iTimerptr = multiTimerCallbackStore[id].callbackID; // set the user var
  // Do not clear anything else in the structure because it may be immediately
  // used if it's part of the fire call. Otherwise we are finished with it.
  // Do not expect there to be an issue with the structure being reused before
  // it gets finally finished being used.
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
  
  if (multiPauseAllTimers) {
    multi_debug("ignoring button up as we are paused\n");
    return;
  }
  multi_debug("multi_button_long_timer_id = %i\n", multiButtonLongTimerID);
  multi_cancel_timer(&multiButtonLongTimerID); // cancel pending mode change
  assert(multiButtonLongTimerID == -1);

  uint32_t downTime=pulse_get_millis() - multiMSAtButtonDown; // how long
  multi_debug("time down was %i\n", downTime);

  // Caused by a mode change? We are now in the new mode so ignore it
  if ( multiCurrentWatchMode != multiLastWatchMode &&
       multiLastWatchMode != -1 ) {  // -1 is no mode previously
    multiLastWatchMode = multiCurrentWatchMode;
    return;
  }

  // Tell the user functions button was released
  multi_watch_functions[multiCurrentWatchMode](BUTTONPRESSED, downTime);
  multi_watch_functions[multiCurrentWatchMode](BUTTONUP);
}

// Main loop. This function is called frequently.
// No blocking calls are allowed in this function or else the watch will reset.
// The inPulse watchdog timer will kick in after 5 seconds if a blocking
// call is made.
void main_app_loop() {
  // get the current time and date
  multi_debug("get date\n");
  pulse_get_time_date(&multiTimeNow);
}


void multi_cancel_all_multi_timers() {
  for (unsigned int id=0; id<MULTI_CALLBACK_STORAGE_SIZE; id++) {
    if (multiTimerCallbackStore[id].userIDVariableLocation) {
      multi_cancel_timer(multiTimerCallbackStore[id].userIDVariableLocation); 
    }
  }
}

// This function is called whenever the processor is about to sleep (to conserve power)
// The sleep functionality is scheduled with pulse_update_power_down_timer(uint32_t)
void main_app_handle_doz() { 
  // Cancel all the users timers (and most of ours)...
  multi_cancel_all_multi_timers();

  // hack the brightness down
  for(int i=MULTI_FADE_MAXIMUM_BRIGHTNESS;i>=MULTI_FADE_MINIMUM_BRIGHTNESS;
      i=i-MULTI_FADE_BRIGHTNESS_STEP) {
    if (i >= MULTI_FADE_MINIMUM_BRIGHTNESS) {
      multi_debug("Bright %i\n", i);
      pulse_oled_set_brightness(i);
      pulse_mdelay(MULTI_FADE_ADJUST_TIME_MS); // faster...
    }
  }
}

void main_app_handle_hardware_update(enum PulseHardwareEvent iEvent) {
  switch(iEvent) {
    case BLUETOOTH_CONNECTED:
      multiBluetoothIsConnected = true; break;
    case BLUETOOTH_DISCONNECTED:
      multiBluetoothIsConnected = false; break;
    default: break; // make compiler happy
  }
}

// Handles bluetooth messages
void multi_received_bluetooth_data(const uint8_t *iBuffer) {
    multi_watch_functions[multiCurrentWatchMode](BLUETOOTHREC, iBuffer);
}


// END
