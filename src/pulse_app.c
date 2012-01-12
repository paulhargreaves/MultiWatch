// Multi-watch engine
// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
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

#include <assert.h>

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
  int32_t idGivenToCaller;
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
bool multiPoweredDown = false; // are we powered down? Used by "sleep" modes, not true powerdown
bool multiPauseAllTimers = false; // if true we are not passing users timer calls back until later
int32_t multiPauseAllTimersTimerID = -1; // used to store the timer for pausing

// Space for our timer callbacks
multiTimerCallbackStruct 
       multiTimerCallbackStore[MULTI_CALLBACK_STORAGE_SIZE]; 

uint32_t multiMSAtButtonDown; // how long was button down for?
int32_t multiButtonLongTimerID = -1; // used to store the long timeout
int32_t multiVibeOnTimerID = -1; // used to store the vibe timeout
int32_t multiTickTockTimerID = -1; // used to store the vibe timeout
int multiCurrentBrightness;
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

  // Call all the modes boot init functions
  for (unsigned int i=0; i<WATCH_MODES; i++) {
    multi_watch_functions[i](COLDBOOT);
  }

  // Switch to the first mode
  multi_change_watch_mode();

  //pulse_update_power_down_timer(MULTI_DEFAULT_POWERDOWN_TIME);
  pulse_register_callback(ACTION_WOKE_FROM_BUTTON, 
                          (PulseCallback) &multi_woke_from_sleep_by_button);
  pulse_register_callback(ACTION_HANDLE_NON_PULSE_PROTOCOL_BLUETOOTH_DATA,
                          (PulseCallback) &multi_received_bluetooth_data);
  multi_debug("*************** main init complete\n");
}


// Just woken up by the button? Call the watch face... will likely redraw
void multi_woke_from_sleep_by_button() {
  multi_debug("woke_from_sleep_by_button in mode %i\n", multiCurrentWatchMode);

  // reset our timers
  multiButtonLongTimerID = -1; 
  multiVibeOnTimerID = -1;
  multiTickTockTimerID = -1;
  multiFadeMaxBrightness = MULTI_FADE_MAXIMUM_BRIGHTNESS;
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
  pulse_oled_set_brightness(multiFadeMaxBrightness); // shouldn't be needed

  // Trigger the tick tock loop for the function
  multi_tick_tock_loop();
}

// Trigger this modes tick tock function if it exists and then
// keep looping. If the mode doesn't need it then we won't keep looping.
void multi_tick_tock_loop() {
  multi_debug("multi_tick_tock_loop\n");
  if (multiLoopTimeMS) {
    multi_debug("calling the watch mode and re-registering\n");
    multi_watch_functions[multiCurrentWatchMode](MAINLOOP);

    multi_cancel_timer(&multiTickTockTimerID);
    assert(multiTickTockTimerID == -1);
    multiTickTockTimerID = 
        multi_register_timer(multiLoopTimeMS,
                             (PulseCallback) &multi_tick_tock_loop, 0); 
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
  multi_cancel_timer(&multiButtonLongTimerID); // might not be running...
  assert(multiButtonLongTimerID == -1);

  multiButtonLongTimerID =
        multi_register_timer(multiModeChangePressTime,
                            (PulseCallback) &multi_change_watch_mode, 0); 

  // reset power down timer to whatever the user set
  multi_update_power_down_timer(multiPowerDownTimeout);
  
  // Tell the user function what just happened
  multi_watch_functions[multiCurrentWatchMode](BUTTONDOWN);
}


// Starts the motor up for a given amount of time
void multi_vibe_for_ms(uint32_t iTimeToVibeForInMS) {
  multi_debug("multi_vibe_for_ms %i\n", iTimeToVibeForInMS);

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

  // Setup a timer to turn the vibe back off. 
  multiVibeOnTimerID = 
      multi_register_timer(iTimeToVibeForInMS,
                           (PulseCallback) &multi_vibe_off, 0); 
  assert(multiVibeOnTimerID != -1);

  // Gentlemen, start your motors!
  pulse_vibe_on();
}
  
// Turns off the motor, called only via multi_vibe_for_ms
void multi_vibe_off() {
  multi_debug("multi_vibe_off()\n");

  pulse_vibe_off();

  multi_cancel_timer(&multiVibeOnTimerID); // Cancel it
  assert(multiVibeOnTimerID == -1); // Timer should be off now
}


void multi_notification_handler_pause_finished() {
  multi_debug("multi_notification_handler_pause_finished\n");

  pulse_cancel_timer(&multiPauseAllTimersTimerID);
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
  multiPauseAllTimers = false;// Release existing timers
  multiYourWatchFaceWasOverwritten = true;
  assert(multiCurrentWatchMode != -1);
  multiPauseAllTimers = true;// Pause existing timers
  
  // Create a 4 second delay
  pulse_cancel_timer(&multiPauseAllTimersTimerID); // pulse
  assert(multiPauseAllTimersTimerID == -1);
  multiPauseAllTimersTimerID = pulse_register_timer(4000, // pulse
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
  pulse_update_power_down_timer(multiPowerDownTimeout);
}

// Register a timer. Stores the return value so we can cancel it when the
// watch mode changes
// There are various magic numbers in here.
// Basic flow: we find an empty slot in our callback structure, then we
//   register a timer and return an id back to the user. The id is calculated
//   as the id slot * 100 + some random - so slot 5 should return ids between
//   500 and 599 (or so).
// The id number is later used in multi_cancel_timer as a way to ensure that
// the person requesting the cancel didn't accidentally cancel someone elses
// timer. E.g. if I gave call1 the ID of 500 and it fires, then we could
// reuse that timer elsewhere. 
int32_t multi_register_timer(uint32_t iTimeoutMS, 
                          PulseCallback iCallback, void *iData) {
  multi_debug("multi_register_timer for %i ms\n", iTimeoutMS);

  int emptySlot = -1; // will contain the slot number
  for (int i = 0; i<MULTI_CALLBACK_STORAGE_SIZE && emptySlot == -1; i++) {
     // Increment the magic variable that helps randomise the ID we return back
     // to the calling function
     multiTimerIDVariable++;
     if (multiTimerIDVariable > 90) { // needs to be less than 99
       multiTimerIDVariable=0;
     }
     if ( multiTimerCallbackStore[i].callbackID == -1 ) {
       emptySlot = i;
       multiTimerCallbackStore[emptySlot].userFunction = iCallback;
       multiTimerCallbackStore[emptySlot].data = iData;
       multiTimerCallbackStore[emptySlot].idGivenToCaller=(emptySlot * 100) +
            multiTimerIDVariable; // 100!
       while (multiTimerCallbackStore[emptySlot].callbackID == -1) {
         multiTimerCallbackStore[emptySlot].callbackID =
               pulse_register_timer(iTimeoutMS,   // pulse!
               (PulseCallback) &multi_timer_fired, (void*)emptySlot);
       }
     }
     multi_debug("call structure %i=%i, user %i\n", i,
                 multiTimerCallbackStore[i].callbackID,
                 multiTimerCallbackStore[i].idGivenToCaller);  
  }
  multi_debug("callback watermark is at %i of %i\n", emptySlot, 
             MULTI_CALLBACK_STORAGE_SIZE-1);
  assert(emptySlot != -1);
    
  return multiTimerCallbackStore[emptySlot].idGivenToCaller; 
}

void multi_timer_fired(void *iData) {
  int id = (int)iData;
  multi_debug("multi_timer_fired %i\n", id);

  // Are we pausing the users timers? If so, just set it up again and
  // return. This must be kept in-step with multi_register_timer
  if (multiPauseAllTimers) {
    multi_debug("pause requested for this id. Recreating.\n");
    pulse_cancel_timer(&multiTimerCallbackStore[id].callbackID);
    multiTimerCallbackStore[id].callbackID =
               pulse_register_timer(500,   // pulse!  And we're waiting 500ms
               (PulseCallback) &multi_timer_fired, (void*)id);
    // we do not touch the rest of the users structure so should be able to
    // call back without any issues
    return;
  }

  // Fire the users function with the data value
  multiTimerCallbackStore[id].userFunction(multiTimerCallbackStore[id].data);
  multi_debug("callback is now %i\n", multiTimerCallbackStore[id].callbackID);
}

void multi_cancel_timer(int32_t *iTimerptr) {
  int userId=*iTimerptr;
  multi_debug("multi_cancel_timer %i\n", userId);

  // No timer specified? Only should happen at first boot...
  if(userId == -1) { return; }; 

  int id=userId / 100; // 100 is magic - see multi_register_timer
  if (multiTimerCallbackStore[id].callbackID == -1) {
    *iTimerptr = -1; // set the user var
    return;
  }
  multi_debug("cancelling timer for id %i user id was %i, should be %i\n", 
              id, userId, multiTimerCallbackStore[id].idGivenToCaller);
  assert(id>=0 && id <MULTI_CALLBACK_STORAGE_SIZE);
  assert(multiTimerCallbackStore[id].callbackID != -1);
  assert(multiTimerCallbackStore[id].idGivenToCaller == userId);

  pulse_cancel_timer(&multiTimerCallbackStore[id].callbackID);
  assert(multiTimerCallbackStore[id].callbackID == -1);
  *iTimerptr = multiTimerCallbackStore[id].callbackID; // set the user var
  multiTimerCallbackStore[id].idGivenToCaller = 9999; // some invalid number
}
  

void main_app_handle_button_up() {
  multi_debug("multi_app_handle_button_up[%i]\n", multiCurrentWatchMode);

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
  // Cancel all timers underneath them... 
  for (unsigned int i=0; i<MULTI_CALLBACK_STORAGE_SIZE; i++) {
    if ( multiTimerCallbackStore[i].callbackID != -1 ) {
       pulse_cancel_timer(&multiTimerCallbackStore[i].callbackID);
       assert(multiTimerCallbackStore[i].callbackID == -1);
       multiTimerCallbackStore[i].idGivenToCaller = 99999;
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
