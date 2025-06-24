#pragma once

#include "esp_log.h"
#include "nvs_flash.h"

#include <string.h>

#define TAG_NVS "NVS"

/// @brief Access the nvs partition where the wifi cred are stored and retrieves them
/// @param ssid A char[32] to pass the ssid once retrieved
/// @param passwd A char[64] to pass the password once retrieved
/// @return True if the credentials were successfully retrieved
bool nvs_Storage_GetWifiCreds(char *ssid, char *passwd);

/// @brief Write on an nvs partition the wifi credential of the AP
/// @param ssid The ssid of the AP
/// @param passwd The password of the AP
esp_err_t nvs_Storage_SetWifiCreds(const char *ssid, const char *passwd);

/// @brief Erase all previous WiFi configuration
esp_err_t nvs_Storage_EraseWifiCreds();
