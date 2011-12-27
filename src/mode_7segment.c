// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// 7-Segment LCD display 
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// 
// Disclaimer: This software is provided "as-is". Use at your own peril

// Below should satisfy Matt's (C) requirement. Mass renaming of objects so
// that they are all (reasonably) unique, plus fit in the framework,
// and some functions shrunk in bytes. Does not attempt to change it to follow
// similar coding standards used elsewhere.


/**
* Simple program to display 7 segment time
*
* Copyright (C) 2011, Matt Shea matt@mattshea.com twitter.com/Matt_Shea
*
* Permission to use, copy, modify, and/or distribute this software for  
* any purpose with or without fee is hereby granted, provided that the  
* above copyright notice and this permission notice appear in all copies.  
*  
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
* WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
* WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
* BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
* OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
* WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   
* ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
* SOFTWARE.
**/


#include "pulse_app.h"
#include "mode_7segment.h"

// How big are we going to make the segment size 
#define SEGMENT_7_WIDTH  16
#define SEGMENT_7_HEIGHT  2*SEGMENT_7_WIDTH
#define SEGMENT_7_THICKNESS 3
#define SEGMENT_7_SPACING 3

static const color24_t MODE_7SEGMENT_COLOR_GREEN24 = {0x00, 0xff, 0x00, 0};
static const color24_t MODE_7SEGMENT_COLOR_DARKGREEN24 = {0x00, 0x0f, 0x00, 0};

//static struct pulse_time_tm last_time;

// Bit fields to specify rendering a segment
#define S_7A 0x1
#define S_7B 0x2
#define S_7C 0x4
#define S_7D 0x8
#define S_7E 0x10
#define S_7F 0x20
#define S_7G 0x40
#define S_7NONE 0x0
#define S_7ALL 0x7F

// Mask lookups for numbers 0-9
static const uint8_t segment_7_mask[10] = { 
        S_7A | S_7B | S_7C | S_7D | S_7E | S_7F, // Digit 0
        S_7B | S_7C,                             // Digit 1
        S_7A | S_7B | S_7G | S_7E | S_7D,        // Digit 2
        S_7A | S_7B | S_7G | S_7C | S_7D,        // Digit 3
        S_7F | S_7B | S_7G | S_7C,               // Digit 4
        S_7A | S_7F | S_7G | S_7C | S_7D,        // Digit 5
        S_7F | S_7G | S_7C | S_7E | S_7D,        // Digit 6
        S_7A | S_7B | S_7C,                      // Digit 7
        S_7ALL,                                  // Digit 8
        S_7A | S_7F | S_7G | S_7B | S_7C         // Digit 9
};
  

void mode_7segment_watch_functions(const enum multi_function_table func, ...) {
  multi_debug("enum %i\n", func);
  switch (func) {
    case MAINLOOP:
      mode_7segment_tick_tock_loop();
      break;
    default: // ignore features we do not use
      break;
  }
}


void draw_7_line (int x, int y, int xlen, int ylen, color24_t color) {
     pulse_set_draw_window(x, y, x+xlen, y+ylen);
     for (uint8_t i=0; i < (xlen+ylen); i++) {
          pulse_draw_point24(color);
     }
}

void draw_7segment_top (uint8_t x, uint8_t y, uint8_t w, color24_t color) {
    for (uint8_t i=0; i < SEGMENT_7_THICKNESS; i++) {
        draw_7_line(x+i, y+i, w - (2*i), 0, color);
    }
}

void draw_7segment_bottom (uint8_t x, uint8_t y, uint8_t w, color24_t color) {
    for (uint8_t i=0; i < SEGMENT_7_THICKNESS; i++) {
        draw_7_line(x+i, y-i, w - (2*i), 0, color);
    }
}


void draw_7segment_left (uint8_t x, uint8_t y, uint8_t h, color24_t color) {
    for (uint8_t i=0; i < SEGMENT_7_THICKNESS; i++) {
        draw_7_line(x+i,y+i, 0, h/2 - (2*i), color);
    }
}

void draw_7segment_right (uint8_t x, uint8_t y, uint8_t h, color24_t color) {
    for (uint8_t i=0; i < SEGMENT_7_THICKNESS; i++) {
        draw_7_line(x-i,y+i, 0, h/2 - (2*i), color);
    }
}

void draw_7segment_middle(uint8_t x, uint8_t y, uint8_t w, color24_t color) {
    draw_7_line(x+1, y, w - 2, 0, color);
    draw_7_line(x, y+1, w , 0, color);
    draw_7_line(x+1, y+2, w - 2, 0, color);
}

void draw_7segment_number(uint8_t val, uint8_t x, uint8_t y, uint8_t w, uint8_t h, color24_t f_color, color24_t b_color) {
    // Sanity check that we only do 1 digit
    val = val % 10;

    // Get the mask of segments    
    uint8_t mask = segment_7_mask[val];

    // Render either the foreground or background (ghost) color 
    // Depending on the mask
    draw_7segment_top(x+1, y, w, (mask & S_7A ? f_color : b_color));

    draw_7segment_right(x + w + 1, y+2, h, (mask & S_7B ? f_color : b_color));
    draw_7segment_right(x + w + 1, y+4+h/2, h,  (mask & S_7C ? f_color : b_color));

    draw_7segment_bottom(x+1,y+h + 4 + 1, w, (mask & S_7D ? f_color : b_color));

    draw_7segment_left(x, y+4+h/2, h, (mask & S_7E ? f_color : b_color));
    draw_7segment_left(x, y+2, h, (mask & S_7F ? f_color : b_color));

    draw_7segment_middle(x+1, y+h/2 + 1, w, (mask & S_7G ? f_color : b_color));

}

void draw_7segment_one(uint8_t val, uint8_t x, uint8_t y, uint8_t h, color24_t f_color, color24_t b_color) {

    draw_7segment_right(x, y+2, h, (val > 0 ? f_color : b_color));
    draw_7segment_right(x, y+4+h/2, h, (val > 0 ? f_color : b_color));

}

void draw_7_colon (uint8_t x, uint8_t y, color24_t color) {
    draw_7_line(x,y-4, 0, 4, color);
    draw_7_line(x+1,y-4, 0, 4, color);
    draw_7_line(x+2,y-4, 0, 4, color);

    draw_7_line(x,y+4, 0, 4, color);
    draw_7_line(x+1,y+4, 0, 4, color);
    draw_7_line(x+2,y+4, 0, 4, color);
}

void draw_7segment_clock(color24_t f_color, color24_t b_color) {
    static struct pulse_time_tm now;
    pulse_get_time_date(&now);

    // Local variable to deal with non-military time display

    if (now.tm_hour > 12) now.tm_hour -= 12; 

    uint8_t y = SCREEN_HEIGHT/2 - SEGMENT_7_HEIGHT/2;
    uint8_t x = SEGMENT_7_SPACING;

    // Draw the "1"
    draw_7segment_one(now.tm_hour / 10, x, y, SEGMENT_7_HEIGHT, f_color, b_color);
    x += SEGMENT_7_THICKNESS + SEGMENT_7_SPACING;

   // Draw the rest of the hour
    draw_7segment_number(now.tm_hour % 10, x, y, SEGMENT_7_WIDTH, SEGMENT_7_HEIGHT, f_color, b_color);
    x += SEGMENT_7_WIDTH + SEGMENT_7_SPACING + 1;

    // Draw the colon
    draw_7_colon(x, y + 1 + SEGMENT_7_HEIGHT/2, (now.tm_sec % 2 == 0 ? f_color : b_color));
    x += SEGMENT_7_THICKNESS + 2;

    // Draw the min
    draw_7segment_number(now.tm_min / 10, x, y, SEGMENT_7_WIDTH, SEGMENT_7_HEIGHT, f_color, b_color);
    x += SEGMENT_7_WIDTH + SEGMENT_7_SPACING + 1;
    draw_7segment_number(now.tm_min % 10, x, y, SEGMENT_7_WIDTH, SEGMENT_7_HEIGHT, f_color, b_color);

    // Draw the sec
    y += SEGMENT_7_HEIGHT/2;
    x += SEGMENT_7_WIDTH + 4;
    draw_7segment_number(now.tm_sec / 10, x, y, SEGMENT_7_WIDTH/2, SEGMENT_7_HEIGHT/2, f_color, b_color);
    x += SEGMENT_7_WIDTH/2 + 4;
    draw_7segment_number(now.tm_sec % 10, x, y, SEGMENT_7_WIDTH/2, SEGMENT_7_HEIGHT/2, f_color, b_color);


}


void mode_7segment_tick_tock_loop(void) {
  draw_7segment_clock(MODE_7SEGMENT_COLOR_GREEN24,
                      MODE_7SEGMENT_COLOR_DARKGREEN24);
}

