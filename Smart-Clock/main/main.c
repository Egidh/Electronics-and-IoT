#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "wifi_manager.h"
#include "wifi_http_server.h"
#include "wifi_captive_portal.h"

#include "clock.h"
#include "lvgl_UI.h"

#define CREDENTIALS_SAVED_BIT BIT0
#define CREDENTIALS_FAILED_BIT BIT1

EventGroupHandle_t wifi_credentials_event;

esp_err_t save_credentials_from_http(const char *ssid, const char *password)
{
    ESP_LOGI("MAIN", "Received WiFi credentials for %s\n", ssid);

    esp_err_t err = nvs_Storage_SetWifiCreds(ssid, password);

    if (err == ESP_OK)
    {
        ESP_LOGI("MAIN", "Succesfully saved WiFi credentials");
        xEventGroupSetBits(wifi_credentials_event, CREDENTIALS_SAVED_BIT);
    }
    else
    {
        ESP_LOGE("MAIN", "Failed to save WiFi credentials");
        xEventGroupSetBits(wifi_credentials_event, CREDENTIALS_FAILED_BIT);
    }

    return err;
}

// static void button_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
// {
//     data->btn_id = 0;
//     data->state = (!gpio_get_level(14)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
// }

// static void switch_menu_task(void *arg)
// {
//     lv_indev_t *button = (lv_indev_t *)arg;

//     while (true)
//     {
//         lv_indev_state_t curr_state = lv_indev_get_state(button);
//         if(curr_state == LV_INDEV_STATE_PRESSED)


//         vTaskDelay(100/portTICK_PERIOD_MS);
//     }
// }

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

    // Initialize netif
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_credentials_event = xEventGroupCreate();

    // Setting up the display
    ui_init();
    lv_obj_t *main_label = NULL;

    lv_style_t *big_style = get_big_label_default_style(LV_ALIGN_CENTER, LV_TEXT_ALIGN_CENTER);
    lv_style_t *mid_style = get_mid_label_default_style(LV_ALIGN_BOTTOM_MID, LV_TEXT_ALIGN_CENTER);

    ui_send_notification("Connecting to internet...", 3000);

    // Connecting to an AP
    esp_netif_t *sta_handle = wifi_init_sta();
    if (sta_handle == NULL)
    {
        wifi_init_ap();
        httpServer_start(save_credentials_from_http);
        DNSserver_StartSocket();

        lv_obj_t *msgbox = ui_message_box_create("Please connect to the WiFi", "SSID : ConnectedClock\nPassword : 12345678");

        while (true)
        {
            EventBits_t bits = xEventGroupWaitBits(
                wifi_credentials_event,
                CREDENTIALS_FAILED_BIT | CREDENTIALS_SAVED_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);

            if (bits & CREDENTIALS_SAVED_BIT)
            {
                ui_delete_obj(msgbox);

                main_label = ui_display_text(main_label, "The smart clock will reboot in 3 seconds...", big_style);

                vTaskDelay(3000 / portTICK_PERIOD_MS);
                esp_restart();
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    // Setting up the clock
    initialize_sntp();

    ui_t *ui = ui_create(true, true);

    // Setting up the push button
    // gpio_config_t config = {
    //     .mode = GPIO_MODE_INPUT,
    //     .pin_bit_mask = (1ULL << GPIO_NUM_14),
    //     .pull_up_en = GPIO_PULLDOWN_ENABLE,
    // };
    // gpio_config(&config);

    // lv_indev_t *button = lv_indev_create();
    // lv_indev_set_type(button, LV_INDEV_TYPE_BUTTON);
    // lv_indev_set_read_cb(button, button_read_cb);
}