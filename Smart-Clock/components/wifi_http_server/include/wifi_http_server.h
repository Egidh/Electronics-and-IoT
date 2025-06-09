// Samuel CHEVIGNARD 2A
// wifi_http_server

#pragma once

#include "esp_http_server.h"
#include "esp_log.h"

#define TAG_HTTP "HTTP_server"

typedef esp_err_t (*wifi_credentials_callback)(const char *ssid, const char *password);

/// @brief Start the http server and set the callback function for the retrieval of the credentials
/// @param func The function which will save the credentials
void httpServer_start(wifi_credentials_callback func);