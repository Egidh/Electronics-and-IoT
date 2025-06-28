#pragma once

#include "esp_sntp.h"
#include "esp_log.h"

#include "lvgl_UI.h"

/// @brief Initialize time using sntp server
void initialize_sntp();

/// @brief Display the actual time on the lcd screen
/// @param pvParameter Unused, freeRTOS task argument requirement
void display_time_task(void *pvParameter);

/// @brief Return the date in the "%a. %d %b" format
/// @param date The string to pass the date to
void myclock_get_date(char *date_str, size_t size);

/// @brief Return the time in the "%H:%M" format
/// @param time_str The string to pass the time to
void myclock_get_time(char *time_str, size_t size);
