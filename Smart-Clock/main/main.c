#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "wifi_manager.h"
#include "wifi_http_server.h"
#include "wifi_captive_portal.h"

#include "display.h"
#include "clock.h"

#define CREDENTIALS_SAVED_BIT BIT0
#define CREDENTIALS_FAILED_BIT BIT1

EventGroupHandle_t wifi_credentials_event;
_lock_t lvgl_api_lock = NULL;

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

void read_button_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (!gpio_get_level(GPIO_NUM_14))
    {
        data->btn_id = 0;
        data->state = LV_INDEV_STATE_PRESSED;
        printf("Button pressed\n");
    }
    else data->state = LV_INDEV_STATE_RELEASED;
}

void button_pressed_cb(lv_event_t *event)
{
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(event);
    char *str = lv_label_get_text(label);

    lv_color_t color = lv_obj_get_style_text_color(label, LV_PART_MAIN);
    const lv_font_t *font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    lv_align_t align = lv_obj_get_style_text_align(label, LV_PART_MAIN);

    lv_display_t *display = lv_obj_get_display(label);
    lcd_display_text(display, label, "Erasing credentials...", font, color, align);

    nvs_Storage_EraseWifiCreds();

    lcd_display_text(display, label, "Credentials successfully erased", font, color, align);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    lcd_display_text(display, label, str, font, color, align);
}

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
    lv_display_t *display = lcd_init();
    lv_obj_t *label = NULL;

    lvgl_api_lock = get_lvgl_api_lock();

    // Connecting to an AP
    esp_netif_t *sta_handle = wifi_init_sta();
    if (sta_handle == NULL)
    {
        wifi_init_ap();
        httpServer_start(save_credentials_from_http);
        DNSserver *server = DNSserver_StartSocket();

        label = lcd_display_text(display, NULL, "Please connect to the WiFi :", &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_CENTER);
        _lock_acquire(&lvgl_api_lock);
        
        lv_obj_t *under_label = NULL;
        under_label = lv_label_create(label);
        lv_obj_set_style_pad_top(under_label, 35, LV_PART_MAIN);
        _lock_release(&lvgl_api_lock);

        under_label = lcd_display_text(display, under_label, "SSID : ConnectedClock\nPassword : 12345678", &lv_font_montserrat_16, lv_color_hex(0xffffff), LV_ALIGN_BOTTOM_MID);

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
                _lock_acquire(&lvgl_api_lock);
                lv_obj_delete(under_label);
                _lock_release(&lvgl_api_lock);

                lcd_display_long_text(display, label, "The smart clock will reboot in 3 seconds...", &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_CENTER);

                vTaskDelay(3000 / portTICK_PERIOD_MS);
                esp_restart();
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    
    // Display info text telling the user how to reset credentials
    lv_obj_t *msg_erase = NULL;
    msg_erase = lcd_display_long_text(display, NULL, "Press the button to erase wifi credentials", &lv_font_montserrat_16, lv_color_hex(0xffffff), LV_ALIGN_BOTTOM_MID);
    _lock_acquire(&lvgl_api_lock);
    lv_obj_set_style_pad_bottom(msg_erase, 16, LV_PART_MAIN);
    _lock_release(&lvgl_api_lock);

    // Displaying WiFi info
    char *buffer = NULL;
    buffer = malloc(64 * sizeof(char));
    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);

    char ssid[33];
    memcpy(ssid, ap_info.ssid, sizeof(ssid));
    sprintf(buffer, "Connected to : %s", ssid);
    printf("%s\n",buffer);
    label = lcd_display_long_text(display, NULL, buffer, &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_CENTER);
}