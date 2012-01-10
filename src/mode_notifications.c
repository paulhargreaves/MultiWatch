// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Notifications watch face
// An example of hooking into the framework but not really using any of it.
// ONLY ONE Notification handler can be compiled into the framework as there
// is only a single callback using pulse_register_callback. If there are
// real-world examples of where multiple handlers are useful then let me know.
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_notifications.h"


// A not very beautiful example of how notifications can be handled
// Probably no need to pause in this function as the function
// multi_external_notification_hander_complete() does that to allow the
// vibrate to occur properly
void mode_notifications_new_notification(PulseNotificationId id) {
  // Clear the screen - it's mine temporarily
  pulse_blank_canvas();
 
  struct PulseNotification *notification = pulse_get_notification(id);
  printf("Type: ", notification->type);
  switch(notification->type) {
    case PNT_MAIL:
      printf("Mail");
      break;
    case PNT_SMS:
      printf("SMS");
      break;
    case PNT_CALENDAR:
      printf("Calendar");
      break;
    case PNT_PHONE:
      printf("Phone");
      break;
    default: // rest are not well described!
      printf("%i - unsure", notification->type);
      break;
  }
    
  printf("\n\n\nMessage:\n\n%s\n\n", notification->sender);
  if (notification->body) { 
    printf("Body:\n%s\n", notification->body);
  }

  // Pass back control to the framework; should be the last thing our
  // handler does. 
  multi_external_notification_handler_complete();
}


// Very simple initialisation - just hook into the one function I want, then
// if I am called as a watch face I'll skip it
void mode_notifications_watch_functions(const enum multi_function_table iFunc,
     ...) {
  multi_debug("enum %i\n", iFunc);
  //va_list varargs;
  //va_start(varargs, iFunc);
  switch (iFunc) {
    case COLDBOOT:
      pulse_register_callback(ACTION_NEW_PULSE_PROTOCOL_NOTIFICATION,
                 (PulseCallback) &mode_notifications_new_notification);
      break;
    case MODEINIT:
      multiSkipThisWatchMode = true; // I am not a real watch face
      break;
    default: // ignore features we do not use
      break;
  }
  //va_end(varargs);
}

