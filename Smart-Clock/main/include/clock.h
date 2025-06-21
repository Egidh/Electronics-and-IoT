#pragma once

#include "esp_sntp.h"
#include "esp_log.h"

#include "display.h"
#include "style.h"

/// @brief Initialize time using sntp server
void initialize_sntp();

/// @brief Display the actual time on the lcd screen
/// @param pvParameter Unused, freeRTOS task argument requirement
void display_time_task(void *pvParameter);
