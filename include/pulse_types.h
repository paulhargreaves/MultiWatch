#ifndef PULSE_TYPES_H
#define PULSE_TYPES_H

#include <stdint.h>

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;  // Not Used
} __attribute__((packed)) color24_t;

/* Some handy colors to use. */
static const color24_t COLOR_BLACK24 = {0, 0, 0, 0};
static const color24_t COLOR_WHITE24 = {0xff, 0xff, 0xff, 0};

// This struct is almost identical to a struct tm time struct (in C), but with some key differences.
// Example: year is not defined as (year - 1900) as it is in struct tm.
struct pulse_time_tm
{
  int32_t tm_sec;			/* Seconds.	[0-60] */
  int32_t tm_min;			/* Minutes.	[0-59] */
  int32_t tm_hour;			/* Hours.	[0-23] */
  int32_t tm_mday;			/* Day.		[1-31] */
  int32_t tm_mon;			/* Month.	[0-11] */
  int32_t tm_year;			/* Year.  */
  int32_t tm_wday;			/* Day of week.	[0-6] */
  int32_t tm_yday;			/* Days in year.[0-365]	*/
  int32_t tm_isdst;			/* DST.		[-1/0/1] <-- not used*/
};

enum PulseHardwareEvent
{
    BLUETOOTH_CONNECTED,
    BLUETOOTH_DISCONNECTED,
    BATTERY_CHARGING,
    BATTERY_NOT_CHARGING,
    NUM_HARDWARE_EVENTS,
};

typedef struct
{
    uint16_t width;
    uint16_t height;
}__attribute__((packed)) pulse_dimensions_t;

/*
 * Callback for the timer system. The parameter is the cookie that was passed in to
 * the pulse_register_timer call.
 */
typedef void (*PulseCallback)(void*);

typedef uint16_t PulseResource;

/* Structure that stores information about a font, it's
 * size, and it's color. */
struct PWFont
{
    PulseResource resource_id;
    color24_t color;
};

typedef char (*WidgetTextDynamicGetChar)(uintptr_t, uint32_t);

enum PWTextStyle {
    PWTS_TRUNCATE = 0x0,
    PWTS_WRAP = 0x1,
    PWTS_WRAP_AT_SPACE = 0x2,  // implies PWTS_WRAP
    PWTS_ELLIPSES = 0x4,
    PWTS_CENTER = 0x8,
    PWTS_VERTICAL_CENTER = 0x10,  // Only applies to truncated text
};

struct PWidgetTextDynamic
{
    WidgetTextDynamicGetChar callback;
    uintptr_t callback_cookie;
    struct PWFont font;
    enum PWTextStyle style;
    uint16_t scroll_offset;         // offset from start of text to draw screen (normally set to 0)
    uint16_t last_char_displayed;
};

struct PWTextBox
{
    uint8_t left;
    uint8_t top;
    uint8_t right;
    uint8_t bottom;
};

enum PulseCallbackAction
{
    ACTION_ALARM_FIRING,
    ACTION_NEW_PULSE_PROTOCOL_NOTIFICATION,
    ACTION_WOKE_FROM_BUTTON,
    ACTION_RETURN_TO_HOME_SCREEN,
    ACTION_HANDLE_NON_PULSE_PROTOCOL_BLUETOOTH_DATA,
    NUM_PULSE_CALLBACK_ACTIONS
};

typedef uint32_t PulseNotificationId;

enum PulseNotificationType
{
    PNT_MAIL = 0,
    PNT_SMS = 1,
    PNT_CALENDAR = 2,
    PNT_PHONE = 3,
    PNT_CALENDAR_BLANK = 4, // Not stored like a regular notification
    PNT_CALENDAR_REMINDER = 5, // Not stored like a regular notification
    PNT_BBM = 6, // Not stored like a regular notification
    PNT_ALARM = 7, // Not stored like a regular notification
    PNT_ALL = 8,
    NUM_NOTIFICATION_TYPES
};

// Maximum notifications stored per type
// These can be accessed using the pulse_get_notification_list_by_type() function
#define MAX_NOTIFICATIONS_PER_LIST (8)

struct PulseNotification
{
    PulseNotificationId id;
    enum PulseNotificationType type;
    const char *sender;
    const char *body;
};

// Alarm-related types

typedef struct
{
    // Type equals 1 for vibrate, others are undefined
    // On and off times specified are in units of 10ms.
    // Example: on1 = 10 will vibrate for 100ms.
    uint8_t type;
    uint8_t vibe_intensity; // not used yet
    uint8_t unused_field2;
    uint8_t unused_field1;
    uint8_t unused_field0;
    uint8_t on1;
    uint8_t off1;
    uint8_t on2;
    uint8_t off2;
    uint8_t on3;
    uint8_t off3;
    uint8_t on4;
} __attribute__((packed)) pp_alert_configuration_t;

typedef struct
{
    uint8_t enabled; // 0 = False, 1 = True, anything else is False
    uint8_t hour; // 0 - 23
    uint8_t min; // 0 - 59
    uint8_t snoozeMin; // Not used
    uint32_t durationMS; // Not used
    pp_alert_configuration_t alert; // Vibe pattern
    uint8_t daysActive[8]; // 0 = Sunday ... 6 = Saturday
    uint8_t msg[32]; // Optional: message
} __attribute__((packed)) PulseAlarm;

#endif
