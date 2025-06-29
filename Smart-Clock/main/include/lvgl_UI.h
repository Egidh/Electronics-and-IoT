#pragma once

#include "lvgl.h"
#include "display.h"
#include "WifiFont.h"
#include "wifi_manager.h"
#include "clock.h"

#include "esp_timer.h"

#define TAG_UI "UI"

#define LCD_WIDTH 320
#define LCD_HEIGHT 172

#define UI_BORDER_SIZE 3

#define LVGL_DRAW_BUF_LINES 20 // number of display lines in each draw buffer
#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

#define UI_MENU_NUM 2

/*****************************************************************************
 *                                 Struct                                    *
 *****************************************************************************/

/// @brief UI object : utilities bar, at the top of the screen
typedef struct ui_top_bar_t
{
    /// @brief The whole content (layout)
    lv_obj_t *content;
    /// @brief The WiFi strength icon
    lv_obj_t *wifi_icon;
    /// @brief The AP ssid
    lv_obj_t *wifi_ssid;
    /// @brief Current date
    lv_obj_t *date;
    /// @brief Current date
    lv_obj_t *time;

} ui_top_bar_t;

/*****************************************************************************
 *                                 Styles                                    *
 *****************************************************************************/

/// @brief Get the default style for big labels
/// @param align The label alignement
/// @param text_align The text alignement
/// @return The style
lv_style_t *get_big_label_default_style(lv_align_t align, lv_text_align_t text_align);

/// @brief Get the default style for mid labels
/// @param align The label alignement
/// @param text_align The text alignement
/// @return The style
lv_style_t *get_mid_label_default_style(lv_align_t align, lv_text_align_t text_align);

/// @brief Get the default style for little labels
/// @param align The label alignement
/// @param text_align The text alignement
/// @return The style
lv_style_t *get_little_label_default_style(lv_align_t align, lv_text_align_t text_align);

/// @brief Get the default style for notifications
/// @return The style
lv_style_t *get_notification_default_style();

/*****************************************************************************
 *                                   UI                                      *
 *****************************************************************************/

/// @brief Initialize the ui
/// @return The main display
lv_display_t *ui_init();

/// @brief Create an UI object : the top bar
/// @return The top bar created
ui_top_bar_t *ui_top_bar_create();

/// @brief Display a text label on the screen
/// @param label The lv_label to use
/// @param text The text you want to display
/// @param style The style of the text
/// @return The label
lv_obj_t *ui_display_text(lv_obj_t *label, const char *text, const lv_style_t *style);

/// @brief Display a notification-like label
/// @param text The text to display
/// @param delay_ms The duration in ms before the notification disappears
void ui_send_notification(const char *text, uint32_t delay_ms);

/// @brief Return the lvgl lock for mutex. Be sure to call ui_init beforehand
_lock_t get_lvgl_api_lock();

/// @brief Create a message box at the center of the display
/// @param title The title of the box
/// @param msg The content
/// @return The box created for further modifications
lv_obj_t *ui_message_box_create(const char *title, const char *msg);

/// @brief Delete an UI object and free its memory
/// @param self The object to be deleted
void ui_delete_obj(lv_obj_t *self);

/// @brief Create an UI object : the clock
/// @param align Where to put the clock
void ui_clock_create(lv_align_t align);
