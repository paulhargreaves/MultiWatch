// watch_config.i

// Add all your mode include file here
#include "mode_demo.h"
#include "mode_powerpoint.h"
#include "mode_nixie.h"
#include "mode_7segment.h"
#include "mode_hourlychime.h"
#include "mode_binary.h"
#include "mode_ryansimple.h"
#include "mode_simple_notifications.h"
#include "mode_displaysleep.h"
#include "mode_sleevepeek.h"
#include "mode_hourlychime2.h"
#include "mode_starkiller.h"
#include "mode_notifications.h"
#include "mode_btlostphone.h"
//#include "mode_notify_history.h"

// Then put in the number of modes, then add your handler function to the
// table below
void (* const multi_watch_functions[])(const enum multi_function_table func, ...) = { 
  //mode_hourlychime_watch_functions,
  mode_hourlychime2_watch_functions,
//  mode_ryansimple_watch_functions,
  // mode_displaysleep_watch_functions,
  // mode_notify_history_watch_functions,
  mode_sleevepeek_watch_functions,
  //mode_binary_watch_functions,
  // mode_nixie_watch_functions,
  // mode_demo_watch_functions,
  // mode_powerpoint_watch_functions,
  // mode_7segment_watch_functions,
  // mode_simple_notifications_watch_functions,
  mode_notifications_watch_functions,
  // mode_starkiller_watch_functions,
  mode_btlostphone_watch_functions,
  NULL // MUST BE LAST ENTRY
};

// end
