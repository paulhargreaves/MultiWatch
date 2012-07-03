// mode_nixie.h
// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//


#ifndef mode_nixie_h
#define mode_nixie_h

void mode_nixie_watch_functions(const enum multi_function_table);

// Woken from sleep
void mode_nixie_woken_by_button(void);
  
// Draw a single digit. 
// digitWanted is 0-9, 10 = space, 11 = "-"
// digitPosition is one of the 4 screen positions:
//     NIXIE_DIGIT_TOPLEFT,    NIXIE_DIGIT_TOPRIGHT,
//     NIXIE_DIGIT_BOTTOMLEFT, NIXIE_DIGIT_BOTTOMRIGHT
// dotWanted is NIXIE_NO_DIGIT_DOT if no dot, NIXIE_YES_DIGIT_DOT if a dot
enum nixie_digit_position {
  NIXIE_DIGIT_TOPLEFT,
  NIXIE_DIGIT_TOPRIGHT,
  NIXIE_DIGIT_BOTTOMLEFT,
  NIXIE_DIGIT_BOTTOMRIGHT
};
#define MODE_NIXIE_DIGIT_POSITION_SIZE 4 // match size of nixie_digit_position!

#define NIXIE_NO_DIGIT_DISPLAYED -1
#define NIXIE_NO_DIGIT_DOT 0  // must be 0
#define NIXIE_YES_DIGIT_DOT 1 // must be 1

// Draw the whole watch face for the current time
void mode_nixie_draw_watch_face(void);

#endif
