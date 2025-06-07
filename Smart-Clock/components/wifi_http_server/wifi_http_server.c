// Samuel CHEVIGNARD 2A
// wifi_http_server

#include "wifi_http_server.h"

static esp_err_t httpServer_FormHandler(httpd_req_t *req)
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

	httpd_resp_send(req, html_form, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static esp_err_t redirect_handler(httpd_req_t *req, httpd_err_code_t err)
{
	httpd_resp_set_status(req, "302 Found");
	httpd_resp_set_hdr(req, "Location", "/");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

void httpServer_start()
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	if (httpd_start(&server, &config) == ESP_OK)
	{
		httpd_uri_t wifi_provisioning_uri = {
			.uri = "/",
			.method = HTTP_GET,
			.handler = httpServer_FormHandler,
			.user_ctx = NULL};
		ESP_ERROR_CHECK(httpd_register_uri_handler(server, &wifi_provisioning_uri));
		ESP_ERROR_CHECK(httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, redirect_handler));

		ESP_LOGI(TAG_HTTP, "HTTP server started on port %d", config.server_port);
	}
	else
		ESP_LOGE(TAG_HTTP, "Failed to start HTTP server!");
}
