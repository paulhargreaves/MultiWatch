// mode_powerpoint.h
// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/

#ifndef mode_powerpoint_h
#define mode_powerpoint_h


void mode_powerpoint_watch_functions(const enum multi_function_table);
void mode_powerpoint_draw_watch_face(void); // draw P logo
void mode_powerpoint_bt_failed(void); // bt message failed :(
void mode_powerpoint_got_bluetooth_data(void); // got a BT message!
void mode_powerpoint_button_pressed(void); // Handling button pressing
void mode_powerpoint_init(void); // Called each time mode is selected
void mode_powerpoint_woken_by_button(void); // Woken from sleep

// How long we wait for a bluetooth response in ms
#define MODE_POWERPOINT_BT_WAIT_TIMEOUT 750 

// bt code to say next slide please! must match python script
#define MODE_POWERPOINT_NEXT_SLIDE 69 
#define MODE_POWERPOINT_PREV_SLIDE 70

// How big our P icon is. Would us pulse_get_image_info but that lot of bytes...
#define MODE_POWERPOINT_IMAGE_WIDTH 48
#define MODE_POWERPOINT_IMAGE_HEIGHT 64

// Other defines
#define MODE_POWERPOINT_VIBE_TIME 25 // how long to vibe for

#endif
