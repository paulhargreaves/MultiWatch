// (C)2011 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
//
// Notifications watch face
// An example of hooking into the framework but not really using any of it.
// 
// Licensed under Creative Commons: Non-Commercial, Share-Alike, Attributation
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril
//

#include "pulse_app.h"
#include "mode_notifications.h"


// A not very beautiful example of how notifications can be handled
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
      
    printf("\n\n\nMessage:\n\n%s", notification->sender);
    //printf("Body:\n%s\n", notification->body);

    pulse_mdelay(5000);

    // Pass back control to the original watch face
    multi_force_refresh_of_watch_face();
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

