// Written by Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// This demo is part of the public domain and can be used for any purpose.
//
// Demo
// 
// Legal disclaimer: This software is provided "as-is". Use at your own peril


#include "pulse_app.h"
#include "mode_demo.h"


/*
enum multi_function_table {
  COLDBOOT, // called on first boot of the watch
  MODEINIT, // called when your mode is asked to init it's globals
  MAINLOOP, // called every X ms where X is controlled by multi_loop_time_ms
  BUTTONWAKE, // called each time the watch wakes from sleep by button
  BLUETOOTHREC, // called each time the watch received bluetooth data
  BUTTONDOWN, // called immediately each time the button is pushed
  BUTTONUP, // called immediately each time the button is released
};
*/

void mode_demo_cold_boot() {
  multi_debug("cold boot\n");
  printf("COLDBOOT\nThe display is not really available...\n");
  pulse_mdelay(1000);
}

void mode_demo_init() {
  multi_debug("init\n");
  multiMyWatchFaceName = "mode_demo";
  multiLoopTimeMS = 1000; // 1 second, since 200ms is too spammy in the demo
  printf("MODEINIT\nThe display is not really available...\n");
  // Would normally not pause here!
  pulse_mdelay(2000);
}

void mode_demo_main_loop() {
  multi_debug("main loop\n");
  printf("MAINLOOP\n");
}

void mode_demo_bluetooth_received() {
  multi_debug("mode_demo_bluetooth_received\n");
  printf("BLUETOOTHREC\nFirst byte was %i\n", multiBluetoothRecBuffer[0]);
}

void mode_demo_button_up() {
  multi_debug("mode_demo_button_up\n");
  printf("BUTTONUP\n");
  printf("Pressed\nFor %i ms\n", multiButtonPressedDownTimeInMS);
}

void mode_demo_button_down() {
  multi_debug("mode_demo_button_down\n");
  printf("BUTTONDOWN\n");
}

void mode_demo_woken_by_button() {
  multi_debug("mode_demo_woken_by_button\n");
  printf("BUTTONWAKE\n");
  printf("My face name %s\n", multiMyWatchFaceName);
}

void mode_demo_watch_functions(const enum multi_function_table iFunc) {
  //multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case COLDBOOT:
      mode_demo_cold_boot();
      break;
    case MODEINIT:
      mode_demo_init();
      break;
    case MAINLOOP:
      mode_demo_main_loop();
      break;
    case BUTTONWAKE:
      mode_demo_woken_by_button();
      break;
    case BUTTONUP:
      mode_demo_button_up();
      break;
    case BUTTONDOWN:
      mode_demo_button_down();
      break;
    case BLUETOOTHREC:
      mode_demo_bluetooth_received();
      break;
    default: // ignore features we do not use
      break;
  }
}

