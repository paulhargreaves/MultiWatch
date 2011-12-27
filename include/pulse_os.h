#ifndef PULSE_OS_H
#define PULSE_OS_H

#include <pulse_types.h>

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * Raw Graphics Functions
 *****************************************************************************/

#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 128

/**
 * Sets the drawing window that any writes to the OLED are written to. After
 * This function is called, the next pixel to be written to the display will
 * be drawn at the location (x1, y1). Any subsequent writes will draw to the
 * right and down until reaching the bottom-right corner of the window, and
 * will wrap back to the top-left corner.
 *
 * Simulator status: Implemented
 *
 * @param x1    The starting (top-left) x-coordinate of the window
 * @param y1    The starting (top-left) y-coordinate of the window
 * @param x2    The ending (bottom-right) x-coordinate of the window
 * @param y2    The ending (bottom-right) y-coordinate of the window
 */
void pulse_set_draw_window(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * Draws a pixel to the screen. Uses 24-bit color. Currently the
 * alpha channel is ignored.
 *
 * Simulator status: Implemented
 *
 * @param color The color24_t to use.
 */
void pulse_draw_point24(color24_t color);

/**
 * Blanks the screen
 *
 * Simulator status: Implemented
 *
 */
void pulse_blank_canvas();

/*****************************************************************************
 * Time and Timer Functions
 *****************************************************************************/

/**
 * Main Timer
 *
 * Simulator status: Implemented
 *
 * @return the time in milliseconds since the processor last dozed (or since boot)
 */
uint32_t pulse_get_millis();

/**
 * Getting the date and time
 *
 * Note: basic date and time information will be populated but anything related to
 *       timezone may not be.
 *
 * Simulator status: Implemented
 *
 * @param tm The struct to be populated with the date and time
 */
void pulse_get_time_date(struct pulse_time_tm *ts);

/**
 * Registers a callback to be notified when at least "timeout" milliseconds elapse
 * with a certain data pointer. An id is returned for this particular registration
 * to be used with pulse_cancel_timer if cancelling a callback is desired.
 *
 * Timers will only expire and callbacks will only be called while the app is
 * waiting for pulse_get_event to return. Apps don't worry about being interupted 
 * by a callback when they're doing other operations.
 *
 * Simulator status: Implemented
 *
 * @param timeout The number of milliseconds the timer will fire after.
 * @param callback The function pointer that gets called after the timer expires.
 * @param data A cookie that will be passed to the callback function as a parameter.
 *
 * @return A unique ID that can be used to cancel the timer later or -1 if timer
 *         can't be registered.
 */
int32_t pulse_register_timer(uint32_t timeout_ms, PulseCallback callback, void *data);

/**
 * Cancelling a timer
 * Note: the int32_t passed in is set to -1 after the timer is cancelled.
 * 
 * Simulator status: Implemented
 *
 * @param id The timer ID returned from pulse_register_timer
 *
 */
void pulse_cancel_timer(int32_t * id);

/*****************************************************************************
 * Power Management
 *****************************************************************************/

/**
 * Sets a timer for processor to enter low power mode
 * 
 * Note: the function main_app_handle_doz() is called before entering low
 *       power mode so that your app is aware that the even is about to occur
 *
 *       Also see pulse_set_bluetooth_sniff() for more power management control
 *
 * Simulator status: Not yet implemented
 *
 * @param schedule_sleep_in_x_ms   Time until the processor should enter low
 *                                 power mode (will be waken by a button press
 *                                 or bluetooth message)
 *
 */
void pulse_update_power_down_timer(uint32_t schedule_sleep_in_x_ms);

/*****************************************************************************
 * Hardware Peripherals
 *****************************************************************************/

/**
 * Turns on the vibrate motor
 *
 * Simulator status: Implemented (turns top menu BLUE when "vibrating")
 */
void pulse_vibe_on(void);

/**
 * Turns off the vibrate motor
 *
 * Simulator status: Implemented
 */
void pulse_vibe_off(void);

/**
 * Sets the oled screen brightness
 * Note: 0 is a dim brightness, but the screen is not turned off.
 *
 * Simulator status: Not yet implemented
 *
 * @param brightness (0 - 100)
 *        
 */
void pulse_oled_set_brightness(uint8_t brightness);

/**
 * Delays for a certain period of time
 *
 * @param ms_to_delay    Number of milliseconds to delay
 *
 * Note: this is a blocking call and blocking for > 5 seconds will
 *       cause the watch to reboot
 *
 * Simulator status: Implemented
 */
void pulse_mdelay(uint32_t ms_to_delay);

/*****************************************************************************
 * Font and Image Rendering Functions
 *****************************************************************************/

/**
 * @return whether an image resource is likely to be valid.
 *
 * Simulator status: Implemented
 */
bool pulse_image_valid(PulseResource f);

/**
 * Retrieves the image dimensions from a pre-stored image
 *
 * Simulator status: Implemented
 *
 * @param image the image to retrieve information about
 * @param width a variable used to return the width of the image
 * @param height a variable used to return the height of the image
 *
 * @return a pulse_dimentions_t type (includes dimensions width and height)
 */
pulse_dimensions_t pulse_get_image_info(PulseResource image);

/**
 * Draws the given image onto the screen at the location specified.
 *
 * @param image                 The PulseResource image to draw to the screen
 * @param x                     The x-value of the location on the screen where the 
 *                              top-left corner of the image should be drawn
 * @param y                     The y-value of the location on the screen where the 
 *                              top-left corner of the image should be drawn
 *
 * Note: follow the tutorial here <<link>> to pack your own images and fonts into
 *       PulseResource format
 *
 * Simulator status: Implemented
 */
void pulse_draw_image(PulseResource image, uint8_t x, uint8_t y);

/**
 * Draws the given image onto the screen at the location specified.
 *
 * @param image                 The PulseResource image to draw to the screen
 * @param x                     The x-value of the location on the screen where the
 *                              top-left corner of the image should be drawn
 * @param y                     The y-value of the location on the screen where the
 *                              top-left corner of the image should be drawn
 * @param transparent_color     The flagged color which you want to make transparent
 *                              (pixels of this color are not written to the screen, 
 *                               so previously drawn pixels "underneath" will still
 *                               be visible)
 *
 * Note: follow the tutorial here <<link>> to pack your own images and fonts into
 *       PulseResource format
 *
 *       The transparency option is useful for drawing overlapping images.
 *       Like the hands of an analog watch face!
 *
 * Simulator status: Implemented
 */
void pulse_draw_image_with_transparency(PulseResource image, uint8_t x, uint8_t y,
        color24_t transparent_color);

/**
 * @return whether a font resource is likely to be valid.
 *
 * Simulator status: Implemented
 */
bool pulse_font_valid(PulseResource f);

/**
 * Initializes a text widget
 *
 * @param widget        Pointer to PWidgetTextDynamic to initialize
 * @param text_buffer   Text buffer to write to the screen
 * @param font          Font to use
 * @param font_color    color of the widget text
 * @param style         Style of text (i.e., truncate, wrap, etc.)
 *                      see code examples for use. Note that style's
 *                      can be OR'd together.
 *                      Example: (PWTS_WRAP_AT_SPACE | PWTS_CENTER)
 * 
 * Simulator status: Implemented
 */
void pulse_init_dynamic_text_widget(struct PWidgetTextDynamic *widget, 
        const char * text_buffer, PulseResource font, color24_t font_color,
        enum PWTextStyle style);

/**
 * Render an initialized text widget to the screen
 *
 * @param draw_context  Pointer to PWTextBox that specifies the bounds
 *                      of the text box
 * @param text_widget   Poitner to an initialized text widget
 *
 * Simulator status: Implemented
 */
void pulse_render_text(struct PWTextBox* draw_context,
        struct PWidgetTextDynamic* text_widget);

/*****************************************************************************
 * Notifications and Bluetooth
 *****************************************************************************/

/**
 * Register to be notified of various OS events like receiving bluetooth data
 * (see pulse_types.h and code examples for details)
 *
 * @param action             The OS event to register
 * @param callback_function  The callback function to call when that event occurs
 *
 * Simulator status: pulse protocol notification callback implemented
 */
void pulse_register_callback(enum PulseCallbackAction action,
        PulseCallback callback_function);

/**
 * If connected by bluetooth, sends a 32-bit integer to the host
 *
 * @param response   The integer to send to the host
 *
 * Simulator status: Not yet implemented
 */
void pulse_send_bluetooth_int(uint32_t response);

/**
 * If connected by bluetooth, send the command to enable sniff mode
 * (for bluetooth power saving)
 *
 * @param sniff_on   Turn sniff mode on or off (true = on)
 * @param sniff_no   The sniff interval. Lower means messages are
 *                   received more often, but uses more power.
 *                   (400 - 2500) is a pretty usable range depending on the
 *                   application.  Sniff mode of 400 will seem near "instant"
 * 
 * Note: if this command is not set, by default the watch uses a sniff_no of
 *       around 2000 
 *
 * Simulator status: Not yet implemented
 */
void pulse_set_bluetooth_sniff(bool sniff_on, uint16_t sniff_no);

/**
 * If you have registered to receive pulse protocol notification
 * (see code examples) this function will let you access the message
 * contents
 *
 * @param id         The id passed to the registered function for pulse protocol
 *                   notifications
 * @return           Pointer to a populated PulseNotification struct
 *                   (see pulse_types.h and code examples for details)
 *
 * Simulator status: Implemented
 */
struct PulseNotification* pulse_get_notification(PulseNotificationId id);

/**
 * Gets a list of recently received notification ids by type
 *
 * @param type       The type of notifications you would like in the list
 *                   (see pulse_types.h and code examples for details)
 * @param list       An array of 8 PulseNotificationIds pre-allocated in memory
 *                   This function will populated this array with
 *                   PulseNotificationIds
 *
 * Simulator status: Implemented
 */
void pulse_get_notification_list_by_type(enum PulseNotificationType type,
        PulseNotificationId list[]);


/*****************************************************************************
 * Alarm 
 *****************************************************************************/

/**
 * Set the alarm
 *
 * Note: this alarm will also wake the processor and call your function
 *       if you registered one with pulse_register_callback() of type
 *       ACTION_ALARM_FIRING. This alarm is persistent across watch
 *       resets.
 *
 * @param alarm_data   Pointer to a populated struct of alarm_data.
 *
 * Note: Internally, this data is copied, so once it is set, this struct
 *       can be discarded. Also, this setting persists across watch
 *       resets.
 *
 * Simulator status: Not Yet Implemented
 */
void pulse_set_alarm(PulseAlarm * alarm_data);

/**
 * Get the active alarm
 *
 * @return           Pointer to a populated PulseAlarm struct
 *
 * Simulator status: Not Yet Implemented
 */
PulseAlarm * pulse_get_alarm();


#ifdef PULSE_SIMULATOR
    /* Override the printf command in the simulator-run apps so that we can print
     * that text to the screen as well as stdout */
    #define printf pulse_printf
#endif

#endif
