#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "lvgl.h"

#define LCD_OFFSET_X 34

#define LCD_WIDTH 320
#define LCD_HEIGHT 172
#define LCD_SPI_HOST SPI2_HOST
#define SPI_CLOCK_HZ (30 * 1000 * 1000)

#define PIN_LCD_MOSI 23
#define PIN_LCD_SCLK 18
#define PIN_LCD_DC 4
#define PIN_LCD_CS 5
#define PIN_LCD_RST 13
#define PIN_LCD_BLK -1

#define LVGL_DRAW_BUF_LINES 20 // number of display lines in each draw buffer
#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

#define TAG_LCD "LCD"

/// @brief Initialize an LCD display in RAM
/// @return A pointer to the lv_display struct
lv_display_t *lcd_init();

/// @brief Display a text on the LCD screen (thread safe)
/// @param self The lv_display to print the text on
/// @param label The lv_label to contain the text, if NULL a label will be created and set to the center of the screen
/// @param text The text to display
/// @param font The font
/// @param color The text color
/// @param align The alignement
/// @return The label passed as an argument or the one created in case the user did not pass any
lv_obj_t *lcd_display_text(lv_display_t *self, lv_obj_t *label, const char *text, const lv_font_t *font, lv_color_t color, lv_align_t align);

/// @brief Return the lvgl lock for mutex. Be sure to call lcd_init beforehand
_lock_t get_lvgl_api_lock();