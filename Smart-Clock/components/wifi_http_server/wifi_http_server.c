// Samuel CHEVIGNARD 2A
// wifi_http_server

#include "wifi_http_server.h"

static wifi_credentials_callback httpServer_getCredentials = NULL;

/// @brief Register a callback function to pass the credentials to the user's code
/// @param func The function to call once the credentials are retrieved
/// @return ESP_ERR_INVALID_ARG if the function isn't correct, ESP_OK if everything went well
static esp_err_t httpServer_registerCredentialCallback(wifi_credentials_callback func)
{
	if (!func)
		return ESP_ERR_INVALID_ARG;

	httpServer_getCredentials = func;

	return ESP_OK;
}

/// @brief URI handler for the captive portal
static esp_err_t httpServer_captiveHandler(httpd_req_t *req)
{
	const char *html_form =
		"<!DOCTYPE html>"
		"<html><head><title>WiFi Setup</title></head><body>"
		"<h2>Configure Wi-Fi</h2>"
		"<form method='POST' action='/submit'>"
		"SSID: <input type='text' name='ssid'><br>"
		"Password: <input type='password' name='password'><br>"
		"<input type='submit' value='Connect'>"
		"</form></body></html>";

	esp_err_t err = httpd_resp_send(req, html_form, HTTPD_RESP_USE_STRLEN);
	if (err != ESP_OK)
		return err;

	return ESP_OK;
}

/// @brief URI handler for the retrieval of the credentials
static esp_err_t httpServer_submitHandler(httpd_req_t *req)
{
	size_t len = req->content_len;
	char ssid[32];
	char password[64];

	char *buffer = malloc(len);
	httpd_req_recv(req, buffer, len);

	httpd_query_key_value(buffer, "ssid", ssid, 32);
	httpd_query_key_value(buffer, "password", password, 64);
	// Debug
	ESP_LOGI(TAG_HTTP, "SSID : %s password : %s", ssid, password);

	free(buffer);

	// Passing the credentials to the callback function
	esp_err_t err = httpServer_getCredentials(ssid, password);
	return err;
}

static esp_err_t redirect_handler(httpd_req_t *req, httpd_err_code_t err)
{
	httpd_resp_set_status(req, "302 Found");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

void httpServer_start(wifi_credentials_callback func)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	if (httpd_start(&server, &config) == ESP_OK)
	{
		httpd_uri_t captive_portal_uri = {
			.uri = "/",
			.method = HTTP_GET,
			.handler = httpServer_captiveHandler,
			.user_ctx = NULL};

		httpd_uri_t submit_uri = {
			.uri = "/submit",
			.method = HTTP_POST,
			.handler = httpServer_submitHandler,
			.user_ctx = NULL};

		ESP_ERROR_CHECK(httpd_register_uri_handler(server, &captive_portal_uri));
		ESP_ERROR_CHECK(httpd_register_uri_handler(server, &submit_uri));
		ESP_ERROR_CHECK(httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, redirect_handler));

		httpServer_registerCredentialCallback(func);

		ESP_LOGI(TAG_HTTP, "HTTP server started on port %d", config.server_port);
	}
	else
		ESP_LOGE(TAG_HTTP, "Failed to start HTTP server!");
}
