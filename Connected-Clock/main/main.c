#include "nvs_flash.h"

#include "wifi_manager.h"
#include "wifi_http_server.h"
#include "wifi_dns_server.h"

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_handle = wifi_init_sta();
    if (sta_handle == NULL)
    {
        wifi_init_ap();
        httpServer_start();
        DNSserver *server = DNSserver_StartSocket();
    }

}