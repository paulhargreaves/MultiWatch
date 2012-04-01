// mode_hourlychime2.h
// (C)2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril

#ifndef mode_hourlychime2_h
#define mode_hourlychime2_h

void mode_hourlychime2_watch_functions(const enum multi_function_table, ...);

// How often we want to alarm? Most of it is hard coded in the .c
// Set to >60 if you only want hourlies
#define MODE_HOURLYCHIME2_SHORT_ALARM 30 

#endif
