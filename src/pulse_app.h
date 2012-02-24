// pulse_app.h
// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Enables (relatively) simple plug-n-play single-tasking multi-watch
// so that the watch will display different faces (modes)
// 
// Disclaimer: This software is provided "as-is". Use at your own peril


#ifndef pulse_app_h
#define pulse_app_h

#include <pulse_os.h>
#include <pulse_types.h>
#include <app_resources.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define null_func 0

// Suppress the build warnings in the simulator and add multi_debug...
#ifdef PULSE_SIMULATOR
  #define DEBUG 1
  int      pulse_printf(const char *, ...); 
#else
  #define DEBUG 0
#endif
int dbg_printf(const char *format, ...);
#define multi_debug(...) do { if (DEBUG) dbg_printf(__VA_ARGS__); } while (0)

// Functions you should examine and call

void multi_cancel_timer(int32_t *); // use instead of pulse_cancel_timer. You only need to call this if you have an outstanding timer - otherwise it is automatically called for you and your variable will automatically be set to -1 when it triggers
void multi_register_timer(int32_t *, uint32_t, PulseCallback, void *); // use instead of pulse_register_timer. NOTE: the parameter list is slightly different - rather than giving a return code, pass in the variable as the first parameter just like multi_cancel_timer. See pulse_app.c for more info.
void multi_update_power_down_timer(uint32_t); // use instead of pulse_update_power_down_timer
void multi_vibe_for_ms(uint32_t); // send time in ms, motor will come on and off automatically

// Specialist things

void multi_external_notification_handler_complete(void); // call only if you have a notifcation when your function has finished. This will allow the existing watch face to rebuild itself as it's likely you have overwritten it with an alert. See mode_notifcations.c for an example.

// Set this in your COLDBOOT if you have decided to write your own power
// handler. Setting this disables the code that automatically adjusts calls
// the watches power down timer so your watch will never automatically
// power down. This will be called frequently. Ideally you should just have
// your own timer defined in here (pulse, not multi) and that should be reset
// on each call so that the watch can do something sensible.
// See mode_displaysleep.c for an example of usage.
void (*multi_external_update_power_down_func)(int); // time in MS

// Set this in your COLDBOOT if you want to run an app while the watch is
// "officially" powered off. See the standard main_app_loop for restrictions
// ONLY one watchface can set it... you mostly should be using the multiwatch
// MAINLOOP function instead
void (*multi_external_main_app_loop_func)(void);

// call this from your sleep function when you are activating your sleep
// function (e.g. pretending to power down)
// This cancels all the existing timers for other watch faces
void multi_external_sleep_init(void); 


// Go to sleep immediately (well, almost...)
void multi_please_sleep_now(int); // int is how long in ms, use 0 or 1 for immediate

// This controls how quickly the loop function runs. You should change it in
// your MODEINIT function if you feel that 200ms is not the right value for your
// watch.
int multiLoopTimeMS;   // is set to 200 by default in pulse_app.c

// Use this to see if bluetooth is connected or not. Even if it is that doesn't
// mean that it's usable
bool multiBluetoothIsConnected;  // READ ONLY - do not set/change

// Use this to see if the watch is currently charging.
bool multiBatteryCharging; // READ ONLY - do not set/change

// Change to adjust the watch time changeover value from the default. If you
// are writing a game you may want a much longer press before it changes, though
// the end user may end up being confused if your mode is substantially longer.
// Remember that after 10 seconds or so the watch will watchdog reboot if the
// user is holding it so you likely want to be quite a bit lower than that.
// Do this in your INIT
int multiModeChangePressTime; // Set in your MODEINIT if you need to change

// Set to true in your MODEINIT function if you do not want this mode to be
// active. This is only really used for modes that just set alarms or monitor
// in the background via their own registered pulse_ timers, and should rarely
// be needed in normal watch modes.
bool multiSkipThisWatchMode; // Set in your MODEINIT if you want it skipped

// Set to true if your watch face is happy that screen writes from other
// watch faces could occur. Normally you can leave this as-is (false) and the
// framework will automatically refresh itself (as if mode had just changed).
// If you are writing a game then you likely want to set this to true and
// you should monitor multiYourWatchFaceWasOverwritten in your main loop -
// as soon as multiYourWatchFaceWasOverwritten becomes true then you know that
// you need to blank the canvas and rebuild the normal users view.
bool multiMyWatchFaceCanHandleScreenOverwrites; // Set in MODEINIT if needed

// Additional note about multiYourWatchFaceWasOverwritten - if you are
// monitoring this flag then whenever you have rebuilt your screen you should
// set this flag to false as the framework will not do it. Also if you have
// not set multiMyFaceCanHandleScreenOverwrites to true then you should
// IGNORE this variable entirly.
bool multiYourWatchFaceWasOverwritten;


// This structure is populated automatically frequently so should be
// reasonably accurate... but is only available after MODEINIT in your watch
// not at COLDBOOT
struct pulse_time_tm multiTimeNow;  // you can change this data in your funcs

// The capabilites that your watch could / should implement
enum multi_function_table {
  COLDBOOT, // called on first boot of the watch
  MODEINIT, // called when your mode is asked to init it's globals
  MAINLOOP, // called every X ms where X is controlled by multi_loop_time_ms
  BUTTONWAKE, // called each time the watch wakes from sleep by button
  BLUETOOTHREC, // called each time the watch received bluetooth data
  BUTTONDOWN, // called immediately each time the button is pushed
  BUTTONUP, // called immediately each time the button is released
  BUTTONPRESSED, // called each time the button is pressed (and released)
};

// COLDBOOT -- called only on first boot of watch. All modes will
// be ran on boot so do not assume you have control of the display etc.
// Rarely needed unless you have a true multitaking app or notifications?

// MODEINIT -- called when the watch changes to your mode. This is where you
// should clear any "global" variables or timer ids you have created, but
// probably not the place to set any new timers - they should be set in
// the BUTTONWAKE section

// BUTTONWAKE -- when the watch powers on by button (and when the watch changes
// mode) the BUTTONWAKE function is called. In here you should set up any
// timers you want to run.

// MAINLOOP -- when the watch is awake will be called every multi_loop_time_ms.
// Used for simpler watches that want the time displayed but aren't ultra
// time sensitive. By default is about every 200ms and in your init function
// you can change it to whatever default you want. Set multi_loop_time_ms to
// 0 if you don't want a loop and you can spare the bytes :-)

// BLUETOOTHREC -- passes bytes from the bluetooth function. Check the
// powerpoint example for details of how to use it.

// BUTTONDOWN -- called immediately as the button is pressed. Unless you are
// writing games you probably want to use BUTTONUP rather than BUTTONDOWN

// BUTTONUP -- called immediately as the button is released.

// BUTTONPRESSED -- called when the button is released along with the amount
// of time in ms that the button was held down for.

#endif
