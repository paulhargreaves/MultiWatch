// (C)2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Alters the time in multitimenow based on the user adjusted input
// Two use cases - all without a phone/computer:
// 1. Time zone adjustment (e.g. I fly to a different country, I adjust my time)
// 2. Lost power, when watch boots it'll be 1st Jan @ 00:00, so adjust 
// 
// This face needs to be the first one running so that it alters time for all
// the others.
//
// Any faces that do not use multiTimeNow will continue to get the (bad?) time
// from pulse_get_time_date and thus should switch if possible...

// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_altertime.h"

void mode_altertime_reset_time(); 
void mode_altertime_adjust_time(); 

//int modeAltertimeTimeAdjustmentMS = 0; // as watch boots, we want it set once
int modeAltertimePointerPos; // position on screen of the * to indicate action

// Places where to put the * on-screen -1 
// Assumption is that the screen printf is every 2 spaces, except for the
// reset button that is 4 places 
#define MODE_ALTERTIME_RESET_POSITION 6 // last thing on the screen

struct modeAltertimeStruct { // from pulse_time_tm
  int32_t tm_min;                       /* Minutes.     [0-59] */
  int32_t tm_hour;                      /* Hours.       [0-23] */
  int32_t tm_mday;                      /* Day.         [1-31] */
  int32_t tm_mon;                       /* Month.       [0-11] */
  int32_t tm_year;                      /* Year.  */
};
struct modeAltertimeStruct modeAltertimeAdjustments;  // persists

const char modeAltertimeDaysOfWeek[][4] = 
                          { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

int modeAltertimeDirection = 1; // persists

void mode_altertime_display() {
  struct pulse_time_tm realWatchTime;
  pulse_get_time_date(&realWatchTime); // We want the real time

  pulse_blank_canvas();

  printf("Hardware time:\n      %2i:%02i\n %s %2i/%2i/%4i\n\n",
         realWatchTime.tm_hour, realWatchTime.tm_min, 
         modeAltertimeDaysOfWeek[realWatchTime.tm_wday], realWatchTime.tm_mday, 
         realWatchTime.tm_mon+1, realWatchTime.tm_year);

  printf("Altered time:\n      %2i:%02i\n %s %2i/%2i/%4i\n\n",
         multiTimeNow.tm_hour, multiTimeNow.tm_min, 
         modeAltertimeDaysOfWeek[multiTimeNow.tm_wday], multiTimeNow.tm_mday, 
         multiTimeNow.tm_mon+1, multiTimeNow.tm_year);

  if ( modeAltertimeDirection == 1) {
    printf("+");
  } else { 
    printf("-");
  }
  printf(" H:M D/M/Y  (R)"); 
  for (int i=0;i < 2*(modeAltertimePointerPos+
              (modeAltertimePointerPos==MODE_ALTERTIME_RESET_POSITION)); i++) {
    printf(" ");
  }
  printf("*\n\n\n\n* = change this");

}

// Change the thing (time) for the individual component pointed at
void mode_altertime_value_change() {
  multi_debug("value change. alterpos=%i\n", modeAltertimePointerPos);
  switch (modeAltertimePointerPos) {
    case 0: // +/-
      modeAltertimeDirection =- modeAltertimeDirection;
      break;
    case 1: // hour
      modeAltertimeAdjustments.tm_hour += modeAltertimeDirection;
      break;
    case 2: // Minute
      modeAltertimeAdjustments.tm_min  += modeAltertimeDirection;
      break;
    case 3: // Day
      modeAltertimeAdjustments.tm_mday += modeAltertimeDirection;
      break;
    case 4: // Month
      modeAltertimeAdjustments.tm_mon  += modeAltertimeDirection;
      break;
    case 5: // Year
      modeAltertimeAdjustments.tm_year += modeAltertimeDirection;
      break;
    case 6: // Reset
      mode_altertime_reset_time();
      break;
    default:
      multi_debug("No idea how to handle %i", modeAltertimePointerPos);
      break;
  } // position on screen of the * to indicate action

  mode_altertime_adjust_time(); // make the change
}

// Resets our structure
void mode_altertime_reset_time() {
  modeAltertimeAdjustments.tm_min  = 0;
  modeAltertimeAdjustments.tm_hour = 0;
  modeAltertimeAdjustments.tm_mday = 0;
  modeAltertimeAdjustments.tm_mon  = 0;
  modeAltertimeAdjustments.tm_year = 0;
}

// Adjusts the time structure globally (multiTimeNow) and corrects the wday etc
void mode_altertime_adjust_time() {
  pulse_get_time_date(&multiTimeNow); // Reset to the current just in case

  // First perform the adjustments
  multiTimeNow.tm_min  += modeAltertimeAdjustments.tm_min;
  multiTimeNow.tm_hour += modeAltertimeAdjustments.tm_hour;
  multiTimeNow.tm_mday += modeAltertimeAdjustments.tm_mday;
  multiTimeNow.tm_mon  += modeAltertimeAdjustments.tm_mon;
  multiTimeNow.tm_year += modeAltertimeAdjustments.tm_year;

  // Now check each variable to make sure it's not over/under
  // Loop, each time we roll over/under we loop again just to make sure
  while (true) {
    if (multiTimeNow.tm_min < 0) {
      multiTimeNow.tm_min += 60;
      multiTimeNow.tm_hour--;
      continue;
    }
    if (multiTimeNow.tm_min > 59) {
      multiTimeNow.tm_min -= 60;
      multiTimeNow.tm_hour++;
      continue;
    }
    if (multiTimeNow.tm_hour < 0) {
      multiTimeNow.tm_hour += 24;
      multiTimeNow.tm_mday--;
      continue;
    }
    if (multiTimeNow.tm_hour > 23 ) {
      multiTimeNow.tm_hour -= 24;
      multiTimeNow.tm_mday++;
      continue;
    }
    if (multiTimeNow.tm_mday < 1) {
      multiTimeNow.tm_mday += 31; // yes... not particularly accurate...
      multiTimeNow.tm_mon--;
      continue;
    }
    if (multiTimeNow.tm_mday > 31 ) { // yes... not particularly accurate...
      multiTimeNow.tm_mday -= 31;
      multiTimeNow.tm_mon++;
      continue;
    }
    if (multiTimeNow.tm_mon < 0) {
      multiTimeNow.tm_mon += 12;
      multiTimeNow.tm_year--;
      continue;
    }
    if (multiTimeNow.tm_mon > 11 ) {
      multiTimeNow.tm_mon -= 12;
      multiTimeNow.tm_year++;
      continue;
    }
    break;
  }
  // Consumes bytes, but just in case the tm_year starts 0...
  if (multiTimeNow.tm_year < 2012) {
    multiTimeNow.tm_year = 2012;
  }
  //multi_debug("?        : min=%i hour=%i mday=%i mon=%i year=%i wday=%i\n", multiTimeNow.tm_min, multiTimeNow.tm_hour, multiTimeNow.tm_mday, multiTimeNow.tm_mon, multiTimeNow.tm_year, multiTimeNow.tm_wday);

  // Now we need to fix wday
  // from https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
  int m = multiTimeNow.tm_mon + 1; 
  int d = multiTimeNow.tm_mday;
  int y = multiTimeNow.tm_year;
  multiTimeNow.tm_wday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;

  // Not fixing yday; this is a "feature" 
}

void mode_altertime_watch_functions(const enum multi_function_table iFunc) {
  //multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case COLDBOOT:
      mode_altertime_reset_time();
      break;
    case APPLOOP:
      mode_altertime_adjust_time();
      // Here is where we set up this special watch mode
      break;
    case MODEINIT:
      multiLoopTimeMS = 6000; // refresh 1 seconds
      modeAltertimePointerPos = 0;
      multiModeChangePressTime = 3000;
      multiButtonDownLongPressTimeMS = 400;
      break;
    case BUTTONDOWNLONGPRESS:
      modeAltertimePointerPos++;
      modeAltertimePointerPos = modeAltertimePointerPos %
                                (MODE_ALTERTIME_RESET_POSITION + 1);
      mode_altertime_display();
      break; 
    case BUTTONUP: 
      mode_altertime_value_change();
      mode_altertime_display();
      break; 
    case MAINLOOP:
      mode_altertime_display();
      break;
    default: // ignore features we do not use
      break;
  }
}

