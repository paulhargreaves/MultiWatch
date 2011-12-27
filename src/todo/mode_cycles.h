// mode_cycles.h

#ifndef mode_cycles_h
#define mode_cycles_h

// Called on first boot
void mode_cycles_main_app_init(void);

// Called each time mode is selected, used to wipe vars
void mode_cycles_init(void);

// Handling button pressing
void mode_cycles_button_pressed(uint32_t);

// Each 1 second tick
void mode_cycles_tick_tock_loop(void);

// Woken from sleep
void mode_cycles_woken_by_button(void);

  
// Draw the whole watch face for the current time
void mode_cycles_draw_watch_face(void);

#endif
