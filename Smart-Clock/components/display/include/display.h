#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_log.h"

#define LCD_SPI_HOST SPI2_HOST
#define SPI_CLOCK_HZ (30 * 1000 * 1000)

#define PIN_LCD_MOSI 23
#define PIN_LCD_SCLK 18
#define PIN_LCD_DC 4
#define PIN_LCD_CS 5
#define PIN_LCD_RST 13
#define PIN_LCD_BLK -1

#define TAG_LCD "LCD"

/// @brief Initialize an st7789v3 display driver
esp_err_t st7789_init(int width, esp_lcd_panel_handle_t *panel_handle);