// watch_config.i

// Add all your mode include file here
#include "mode_demo.h"
#include "mode_powerpoint.h"
#include "mode_nixie.h"
#include "mode_7segment.h"
#include "mode_hourlychime.h"
#include "mode_binary.h"
#include "mode_ryansimple.h"
#include "mode_notifications.h"
#include "mode_displaysleep.h"
#include "mode_sleevepeek.h"

// Then put in the number of modes, then add your handler function to the
// table below

#define WATCH_MODES 7

// Strange error about excess elements? Check your WATCH_MODES above matches
// number of entries in the multi_watch_functions
// Random crashes may occur if you comment out functions but do not reduce
// WATCH_MODES...
void (* const multi_watch_functions[WATCH_MODES])(const enum multi_function_table func, ...) = { 
  //mode_hourlychime_watch_functions,
  //mode_ryansimple_watch_functions,
  //mode_displaysleep_watch_functions,
  mode_sleevepeek_watch_functions,
  mode_binary_watch_functions,
  mode_nixie_watch_functions,
  mode_demo_watch_functions,
  mode_powerpoint_watch_functions,
  mode_7segment_watch_functions,
  mode_notifications_watch_functions,
};

// end
