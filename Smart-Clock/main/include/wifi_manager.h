#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "sys/socket.h"

#include <string.h>

#include "nvs_storage.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define MAX_RETRY 3

#define TAG_STA "Station"
#define TAG_AP "AP"

/// @brief Be sure to call nvs_flash_init() before calling this function
/// @return 0, if the Wi-Fi credentials have been successfully loaded from NVS. 1 otherwise.
esp_netif_t *wifi_init_sta();

esp_netif_t *wifi_init_ap();

/// @brief Return the ssid of the current AP
/// @param ssid A string to pass the ssid (usually the ssid is 34 bytes long)
/// @param size The size of the ssid buffer 
void wifi_get_SSID(char *ssid, size_t size);
