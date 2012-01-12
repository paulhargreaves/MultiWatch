// mode_7segment.h
// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/


#ifndef mode_7segment_h
#define mode_7segment_h

void mode_7segment_watch_functions(const enum multi_function_table, ...);

// Each 1 second tick
void mode_7segment_tick_tock_loop(void);

#endif
