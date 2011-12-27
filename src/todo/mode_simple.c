// Multi-watch engine
// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Cycles
// 
// Legal disclaimer: This software is provided "as-is". Use at your own peril

#include "pulse_app.h"
#include "mode_simple.h"


static const PulseResource mode_simple_month_text[] = {
  IMAGE_MODE_SIMPLE_MONTH_0_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_1_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_2_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_3_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_4_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_5_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_6_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_7_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_8_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_9_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_10_SMALL,
  IMAGE_MODE_SIMPLE_MONTH_11_SMALL
};

static const PulseResource mode_simple_watch_digits[] = {
  IMAGE_MODE_SIMPLE_0_BIG,
  IMAGE_MODE_SIMPLE_1_BIG,
  IMAGE_MODE_SIMPLE_2_BIG,
  IMAGE_MODE_SIMPLE_3_BIG,
  IMAGE_MODE_SIMPLE_4_BIG,
  IMAGE_MODE_SIMPLE_5_BIG,
  IMAGE_MODE_SIMPLE_6_BIG,
  IMAGE_MODE_SIMPLE_7_BIG,
  IMAGE_MODE_SIMPLE_8_BIG,
  IMAGE_MODE_SIMPLE_9_BIG,
  IMAGE_MODE_SIMPLE_0_SMALL,
  IMAGE_MODE_SIMPLE_1_SMALL,
  IMAGE_MODE_SIMPLE_2_SMALL,
  IMAGE_MODE_SIMPLE_3_SMALL,
  IMAGE_MODE_SIMPLE_4_SMALL,
  IMAGE_MODE_SIMPLE_5_SMALL,
  IMAGE_MODE_SIMPLE_6_SMALL,
  IMAGE_MODE_SIMPLE_7_SMALL,
  IMAGE_MODE_SIMPLE_8_SMALL,
  IMAGE_MODE_SIMPLE_9_SMALL,
  IMAGE_MODE_SIMPLE_COLON_BIG, // 20 - :
  IMAGE_MODE_SIMPLE_ST_SMALL, // 21 - "st"  as in 1st Mar
  IMAGE_MODE_SIMPLE_ND_SMALL, // 22 - "nt"  as in 2nd Apr
  IMAGE_MODE_SIMPLE_TH_SMALL // 23 - "th" as in 18th Feb
};

void mode_simple_draw_single_digit(int digitWanted, int xposWanted, bool small) {
  #define X_OFFSET 3
  #define Y_OFFSET 33
  int myDigit = digitWanted;
  int ypos=0;
  if (small) {
     myDigit += 10;
     ypos=40;
  }
  debug_print("printing image %i at %i,%i",myDigit,xposWanted+X_OFFSET,ypos+Y_OFFSET);
  pulse_draw_image(mode_simple_watch_digits[myDigit],
                   xposWanted+X_OFFSET, ypos+Y_OFFSET);
}

void mode_simple_draw_watch_face() {
  struct pulse_time_tm now;
  pulse_get_time_date(&now);
  mode_simple_draw_single_digit(now.tm_hour / 10, 0, false);
  mode_simple_draw_single_digit(now.tm_hour % 10, 20, false);
  mode_simple_draw_single_digit(20, 43, false); // 20 is colon
  mode_simple_draw_single_digit(now.tm_min / 10, 52, false);
  mode_simple_draw_single_digit(now.tm_min % 10, 72, false); 
  debug_print("simple %i:%i:%i\n", now.tm_hour, now.tm_min, now.tm_sec);

/*
  if (now.tm_mday > 10) {
    mode_simple_draw_single_digit(now.tm_mday / 10, 2, true); 
  }
  mode_simple_draw_single_digit(now.tm_mday % 10, 9, true); 
  int suffix;
  switch (now.tm_mday) {
    case 1:
    case 21:
    case 31:
      suffix=21; break;
    case 2:
    case 22:
      suffix=22; break;
    default:
      suffix=23; break;
  }
  mode_simple_draw_single_digit(suffix-10, 16, true); 

  pulse_draw_image(mode_simple_month_text[now.tm_mon],
                   33, 73);
*/

 printf("\n\n\n\n\n\n\n\n\n\n   %i of %i", now.tm_mday, now.tm_mon+1);
  
}

