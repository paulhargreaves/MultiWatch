// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Ryan's "simple" clock
// Editing done to fit framework and ensure variables/functions should not
// clash with other modes.
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

/*
* Simple Analog Clock
*
* Description:
* Draws an analog clock using the watch's native graphics. Single pixel thick anti-aliased
* lines are drawn using Xiaolin Wu's algorithm. The processor will go to sleep in 15 seconds
* if there is no activity. Clicking the button will animate the time.
*
*
* Copyright (C) 2011, Allerta Inc.
* Author: Ryan Young (ryan@allerta.ca)
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
*/

#include "pulse_app.h"
#include "mode_ryansimple.h"

void mode_ryansimple_draw_time();
void mode_ryansimple_draw_spinning_time();
uint8_t mode_ryansimple_current_min;
uint8_t mode_ryansimple_current_hour;

static const color24_t mode_ryansimple_hand_color = {113, 217, 226, 0};
static const color24_t mode_ryansimple_tick_color = {0, 123, 167, 0};

typedef struct mode_ryansimple_point
{
  uint8_t x;
  uint8_t y;
} mode_ryansimple_point;

//mode_ryansimple_point center = {47, 63};
#define MODE_RYANSIMPLE_POINT_CENTER_X  47
#define MODE_RYANSIMPLE_POINT_CENTER_Y  63

static const mode_ryansimple_point mode_ryansimple_Rtick[] =
{
    {46, 16}, {48, 16}, {70, 23}, {87, 40},
    {94, 62}, {94, 64}, {87, 86}, {70, 103},
    {48, 110}, {46, 110}, {24, 103}, {7, 86},
    {0, 64}, {0, 62}, {7, 40}, {24, 23}
};

static const mode_ryansimple_point mode_ryansimple_rtick[] =
{
    {46, 21}, {48, 21}, {67, 28}, {82, 43},
    {89, 62}, {89, 64}, {82, 83}, {67, 98},
    {48, 105}, {46, 105}, {27, 98}, {12, 83},
    {5, 64}, {5, 62}, {12, 43}, {27, 28}
};

static const mode_ryansimple_point mode_ryansimple_min_hand[] =
{
    {47, 24}, {51, 24}, {55, 25}, {59, 26}, {63, 28},
    {66, 30}, {69, 32}, {72, 34}, {75, 37}, {78, 40},
    {80, 44}, {82, 48}, {84, 52}, {85, 56}, {86, 60},
    {86, 63}, {86, 66}, {85, 70}, {84, 74}, {82, 78},
    {80, 82}, {78, 85}, {75, 88}, {72, 91}, {69, 94},
    {66, 96}, {62, 98}, {58, 99}, {55, 100}, {51, 101},
    {47, 101}, {43, 101}, {39, 100}, {36, 99}, {32, 98},
    {28, 96}, {25, 94}, {22, 91}, {19, 88}, {16, 85},
    {14, 82}, {12, 78}, {10, 74}, {9, 70}, {8, 66},
    {8, 63}, {8, 60}, {9, 56}, {10, 52}, {12, 48},
    {14, 44}, {16, 40}, {18, 37}, {21, 34}, {24, 32},
    {28, 30}, {31, 28}, {35, 26}, {39, 25}, {43, 24}
};

static const mode_ryansimple_point mode_ryansimple_hour_hand[] =
{
    {47, 35}, {51, 35}, {54, 36}, {58, 37},
    {61, 39}, {64, 41}, {67, 43}, {69, 46},
    {71, 49}, {73, 52}, {74, 56}, {75, 59},
    {75, 63}, {75, 66}, {74, 69}, {73, 73},
    {71, 77}, {69, 80}, {67, 83}, {64, 85},
    {61, 87}, {58, 89}, {54, 90}, {51, 91},
    {47, 91}, {44, 91}, {41, 90}, {37, 89},
    {33, 87}, {30, 85}, {27, 83}, {24, 81},
    {22, 78}, {20, 75}, {19, 71}, {18, 67},
    {18, 63}, {18, 59}, {19, 56}, {20, 52},
    {22, 49}, {24, 46}, {27, 43}, {30, 40},
    {33, 38}, {36, 37}, {40, 36}, {43, 35}
};

color24_t mode_ryansimple_adjustintensity(color24_t color, uint8_t intensity)
{
    color24_t temp = {color.red * intensity / 255,
                      color.green * intensity / 255,
                      color.blue * intensity / 255, 0};
    return temp;
}

void mode_ryansimple_swap(uint8_t *a, uint8_t *b)
{
    uint8_t temp = *a;
    *a = *b;
    *b = temp;
}

void mode_ryansimple_watch_functions(const enum multi_function_table iFunc, ...) {
  multi_debug("enum %i\n", iFunc);
  //va_list varargs;
  //va_start(varargs, iFunc);
  switch (iFunc) {
    /*case MODEINIT:
      mode_ryansimple_current_min = 0;
      mode_ryansimple_current_hour = 0;
      break;
    */
    case BUTTONWAKE:
      mode_ryansimple_draw_time();
      break;
    case BUTTONUP:
      mode_ryansimple_draw_spinning_time();
      break;
    default: // ignore features we do not use
      break;
  }
  //va_end(varargs);
}


void mode_ryansimple_draw_wu_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, color24_t color)
{
    //Draw 2 endpoints
    pulse_set_draw_window(x0, y0, x0, y0);
    pulse_draw_point24(color);
    pulse_set_draw_window(x1, y1, x1, y1);
    pulse_draw_point24(color);

    //Negative slope
    bool negative = (y0 > y1) ? true : false;

    if(negative)
    {
        mode_ryansimple_swap(&y0, &y1);
    }
    if(x0 > x1)
    {
        mode_ryansimple_swap(&x0, &x1);
        negative ^= true;
    }
    uint8_t dx = x1 - x0;
    uint8_t dy = y1 - y0;

    if(!dx) //Vertical line
    {
        pulse_set_draw_window(x0, y0 + 1, x0, y1 - 1);
        for(int i = 1; i < dy; i++)
        {
            pulse_draw_point24(color);
        }
    }
    else if(!dy) //Horizontal line
    {
        pulse_set_draw_window(x0 + 1, y0, x1 - 1, y0);
        for(int i = 1; i < dx; i++)
        {
            pulse_draw_point24(color);
        }
    }
    else if(dx == dy) //45 degree line
    {
        unsigned char dy = (negative) ? -1 : 1;
        unsigned y = (negative) ? y1 : y0;
        for(int i = 1; i < dx; i++)
        {
            x0++;
            y += dy;
            pulse_set_draw_window(x0, y, x0, y);
            pulse_draw_point24(color);
        }
    }
    else //Other line
    {
        //Slope is > 1 or not
        bool steep  = (dy > dx) ? true : false;
        if(steep)
        {
            mode_ryansimple_swap(&x0, &y0);
            mode_ryansimple_swap(&x1, &y1);
            mode_ryansimple_swap(&dx, &dy);
        }

        //Distance from ideal pixel center
        uint8_t D = 0;

        //Stores last pixel's distance
        uint8_t lastD = 0;

        //Distance increment
        uint8_t d = dy * 256 / dx;

        //Temp coordinates used to plot pixels
        uint8_t a0, a1, b0, b1;

        //Temp coordinate offset
        uint8_t da, db;

        while(x0 < x1)
        {
            x0++;;
            x1--;
            D += d;

            //Check for overflow
            if(lastD > D)
            {
                y0++;
                y1--;
            }
            lastD = D;
            a0 = x0;
            b0 = y0;
            a1 = x1;
            b1 = y1;
            da = 0;
            db = 1;
            if(negative & ~steep)
            {
                mode_ryansimple_swap(&a0, &a1);
            }
            else if(negative & steep)
            {
                mode_ryansimple_swap(&a0, &b0);
                mode_ryansimple_swap(&a1, &b1);
                mode_ryansimple_swap(&b0, &b1);
                mode_ryansimple_swap(&da, &db);
            }
            else if(~negative & steep)
            {
                mode_ryansimple_swap(&a0, &b0);
                mode_ryansimple_swap(&a1, &b1);
                mode_ryansimple_swap(&da, &db);
            }

            //Draw 4 points
            pulse_set_draw_window(a0, b0, a0, b0);
            pulse_draw_point24(mode_ryansimple_adjustintensity(color, ~D));
            pulse_set_draw_window(a0 + da, b0 + db, a0 + da, b0 + db);
            pulse_draw_point24(mode_ryansimple_adjustintensity(color, D));
            pulse_set_draw_window(a1, b1, a1, b1);
            pulse_draw_point24(mode_ryansimple_adjustintensity(color, ~D));
            pulse_set_draw_window(a1 - da, b1 - db, a1 - da, b1 - db);
            pulse_draw_point24(mode_ryansimple_adjustintensity(color, D));
        }
    }
}

void mode_ryansimple_draw_hands()
{
    mode_ryansimple_draw_wu_line(MODE_RYANSIMPLE_POINT_CENTER_X, MODE_RYANSIMPLE_POINT_CENTER_Y, mode_ryansimple_min_hand[mode_ryansimple_current_min].x, mode_ryansimple_min_hand[mode_ryansimple_current_min].y, mode_ryansimple_hand_color);
    mode_ryansimple_draw_wu_line(MODE_RYANSIMPLE_POINT_CENTER_X, MODE_RYANSIMPLE_POINT_CENTER_Y, mode_ryansimple_hour_hand[4 * (mode_ryansimple_current_hour % 12) + mode_ryansimple_current_min / 15].x,
            mode_ryansimple_hour_hand[4 * (mode_ryansimple_current_hour % 12) + mode_ryansimple_current_min / 15].y, mode_ryansimple_hand_color);
}

void mode_ryansimple_erase_hands()
{
    mode_ryansimple_draw_wu_line(MODE_RYANSIMPLE_POINT_CENTER_X, MODE_RYANSIMPLE_POINT_CENTER_Y, mode_ryansimple_min_hand[mode_ryansimple_current_min].x, mode_ryansimple_min_hand[mode_ryansimple_current_min].y, COLOR_BLACK24);
    mode_ryansimple_draw_wu_line(MODE_RYANSIMPLE_POINT_CENTER_X, MODE_RYANSIMPLE_POINT_CENTER_Y, mode_ryansimple_hour_hand[4 * (mode_ryansimple_current_hour % 12) + mode_ryansimple_current_min / 15].x,
            mode_ryansimple_hour_hand[4 * (mode_ryansimple_current_hour % 12) + mode_ryansimple_current_min / 15].y, COLOR_BLACK24);
}

void mode_ryansimple_draw_time()
{
    //pulse_cancel_timer(&sleep_timer);
    //pulse_get_time_date(&mode_ryansimple_current_time);
    mode_ryansimple_erase_hands();
    mode_ryansimple_current_hour = multiTimeNow.tm_hour;
    mode_ryansimple_current_min = multiTimeNow.tm_min;
    /*
    if(mode_ryansimple_current_hour != mode_ryansimple_current_time.tm_hour)
    {
        mode_ryansimple_current_hour = mode_ryansimple_current_time.tm_hour;
    }
    if(mode_ryansimple_current_min != mode_ryansimple_current_time.tm_min)
    {
        mode_ryansimple_current_min = mode_ryansimple_current_time.tm_min;
    }
    */
    mode_ryansimple_draw_hands();
    for(int i = 0; i < 16; i++)
    {
        mode_ryansimple_draw_wu_line(mode_ryansimple_Rtick[i].x, mode_ryansimple_Rtick[i].y, mode_ryansimple_rtick[i].x, mode_ryansimple_rtick[i].y, mode_ryansimple_tick_color);
    }
    //sleep_timer = pulse_register_timer(SLEEP_TIMEOUT, &pulse_update_power_down_timer, 0);
}

void mode_ryansimple_draw_spinning_time()
{
    mode_ryansimple_erase_hands();
    //pulse_get_time_date(&mode_ryansimple_current_time);
    uint8_t temp_min = multiTimeNow.tm_min;
    uint8_t temp_hour = multiTimeNow.tm_hour % 12;
    mode_ryansimple_current_min = 0;
    mode_ryansimple_current_hour = 0;
    while(temp_min != mode_ryansimple_current_min || temp_hour != mode_ryansimple_current_hour)
    {
        mode_ryansimple_draw_hands();
        mode_ryansimple_erase_hands();
        mode_ryansimple_current_min++;
        if(mode_ryansimple_current_min == 60)
        {
            mode_ryansimple_current_min = 0;
            mode_ryansimple_current_hour++ % 12;
        }
    }
    mode_ryansimple_draw_hands();
}
