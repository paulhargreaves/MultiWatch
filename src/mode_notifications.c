// (C)2011-2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
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

//char   mode_notifications_body_text[450]; // magic 
struct PWTextBox mode_notifications_text_box;
struct PWidgetTextDynamic mode_notifications_text_widget;

void mode_notifications_print(enum PWTextStyle style, const char* iText) {
  assert(mode_notifications_text_box.top >= 0);
  assert(mode_notifications_text_box.bottom <= SCREEN_HEIGHT - 1);
  assert(mode_notifications_text_box.left >= 0);
  assert(mode_notifications_text_box.right <= SCREEN_WIDTH - 1);

  pulse_init_dynamic_text_widget(&mode_notifications_text_widget, 
                                 //mode_notifications_body_text,
                                 iText,
           FONT_GEORGIA_10, COLOR_WHITE24, style);

  //sprintf(mode_notifications_body_text,"%s", iText); 

  pulse_render_text(&mode_notifications_text_box,
                    &mode_notifications_text_widget);
}

// Called when we want a notification to be displayed
void mode_notifications_new_notification(PulseNotificationId id) {
  mode_notifications_text_box.right = SCREEN_WIDTH - 1;
  mode_notifications_text_box.left = 0;

  struct PulseNotification *notification = pulse_get_notification(id);
  if (notification->type == PNT_PHONE) {
    multi_debug("Phone\n");
    // Do phone stuff
    pulse_draw_image(IMAGE_MODE_NOTIFICATIONS_PHONE, 32, 20);
    mode_notifications_text_box.top = 60;
    assert(mode_notifications_text_box.left == 0);
    mode_notifications_text_box.bottom = SCREEN_HEIGHT - 1;
    mode_notifications_print(PWTS_CENTER, notification->sender);

    mode_notifications_text_box.top = 76;
    mode_notifications_print(PWTS_CENTER, notification->body);

    return;
  } else { // Not a phone message
    switch(notification->type) {
      case PNT_MAIL:
        pulse_draw_image(IMAGE_MODE_NOTIFICATIONS_EMAIL_ENVELOPE, 0, 3);
        multi_debug("Mail\n");
        break;
      case PNT_SMS:
        multi_debug("SMS\n");
        // break; // FALLS THROUGH TO PNT_CALENDAR so they use the same icon
      case PNT_CALENDAR:
        pulse_draw_image(IMAGE_MODE_NOTIFICATIONS_CHAT_TALK_SPEAK, 0, 3);
        multi_debug("Calendar\n");
        break;
      default: // rest are not well described!
        //printf("No idea");
        break;
    }

    // Common notification display format

    // Body
    assert(mode_notifications_text_box.right == SCREEN_WIDTH - 1);
    assert(mode_notifications_text_box.left == 0);
    mode_notifications_text_box.top = 16;
    mode_notifications_text_box.bottom = SCREEN_HEIGHT - 1;
    mode_notifications_print(PWTS_WRAP_AT_SPACE, notification->body);

    // Sender
    assert(mode_notifications_text_box.right == SCREEN_WIDTH - 1);
    mode_notifications_text_box.top = 0;
    mode_notifications_text_box.left = 20;
    mode_notifications_text_box.bottom = 14;
    mode_notifications_print(PWTS_WRAP_AT_SPACE, notification->sender);
  }
    
  // Pass back control to the framework; should be the last thing our
  // handler does. 
  multi_external_notification_handler_complete();
}


// Very simple initialisation - just hook into the one function and
// if called as a watch face, skip it
void mode_notifications_watch_functions(const enum multi_function_table iFunc) {
  //multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case COLDBOOT:
      multi_register_notifications(
              (PulseCallback) &mode_notifications_new_notification);
      break;
    case MODEINIT:
      multiSkipThisWatchMode = true; // I am not a real watch face
      break;
    default: // ignore features we do not use
      break;
  }
}

