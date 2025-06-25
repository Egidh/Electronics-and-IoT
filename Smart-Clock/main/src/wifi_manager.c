#include "wifi_manager.h"

static EventGroupHandle_t wifi_event_group;

static int retry_count = 0;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
            esp_wifi_connect();
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGI("Station", "Disconnected from AP.\n");
            if (retry_count < MAX_RETRY)
            {
                esp_wifi_connect();
                retry_count++;
                ESP_LOGI("Station", "Trying to reconnect..\n %d try out of %d\n", retry_count, MAX_RETRY);
            }
            else
            {
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGI("Station", "Connection failed.\n");
            }
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI("Station", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_StopSta(esp_netif_t *sta_handle,
                         esp_event_handler_instance_t instance_any_id,
                         esp_event_handler_instance_t instance_got_ip)
{
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    esp_netif_destroy(sta_handle);

    vEventGroupDelete(wifi_event_group);
    wifi_event_group = NULL;
}

esp_netif_t *wifi_init_sta()
{
    char ssid[32];
    char passwd[64];

    bool gotCredentials = nvs_Storage_GetWifiCreds(ssid, passwd);
    if (!gotCredentials)
    {
        ESP_LOGI(TAG_STA, "No credentials found !");
        return NULL;
    }

    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL)
    {
        ESP_LOGE(TAG_STA, "Failed to create EventGroup");
        return NULL;
    }

    esp_netif_t *sta_handle = esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_init_config_t w_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&w_init_config));

    wifi_config_t w_config = {0};
    strncpy((char *)w_config.sta.ssid, ssid, sizeof(w_config.sta.ssid));
    strncpy((char *)w_config.sta.password, passwd, sizeof(w_config.sta.password));
    w_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &w_config)); // Need to handle this differently because it reboot if the AP isn't reachable or if the credentials are wrong
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_FAIL_BIT | WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI("Station", "Connected to %s.\n", w_config.sta.ssid);
        return sta_handle;
    }
    else if (bits & WIFI_FAIL_BIT)
        ESP_LOGI("Station", "Failed to connect to %s.\n", w_config.sta.ssid);
    else
        ESP_LOGE("Station", "Unknown event.\n");

    wifi_StopSta(sta_handle, instance_any_id, instance_got_ip);

    return NULL;
}

esp_netif_t *wifi_init_ap()
{
    esp_netif_t *ap_handle = esp_netif_create_default_wifi_ap();

    wifi_init_config_t w_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&w_init_config));

    wifi_config_t w_config = {
        .ap = {
            .ssid = "ConnectedClock",
            .ssid_len = strlen("ConnectedClock"),
            .channel = 1,
            .password = "12345678",
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {.required = false},
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &w_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    char *portalURI = "http://192.168.4.1";

    ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_handle));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(ap_handle, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI, portalURI, strlen(portalURI)));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_handle));

    return ap_handle;
}

void wifi_get_SSID(char *ssid, size_t size)
{
    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);

    strlcpy(ssid, (char *)ap_info.ssid, size);
}
