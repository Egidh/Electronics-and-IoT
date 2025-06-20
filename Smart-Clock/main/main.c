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

static TimerHandle_t debounce_timer = NULL;
static bool button_enabled = true;
static void debounce_timer_cb(TimerHandle_t timer)
{
    button_enabled = true;
}

static QueueHandle_t erase_wifi_queue = NULL;
static void IRAM_ATTR button_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (button_enabled)
    {
        uint8_t signal = 1; // Signal d'effacement
        xQueueSendFromISR(erase_wifi_queue, &signal, &xHigherPriorityTaskWoken);
        button_enabled = false;
        xTimerStartFromISR(debounce_timer, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Tâche pour effacer les credentials
void erase_wifi_task(void *arg)
{
    lv_display_t *display = (lv_display_t *)arg;
    uint8_t signal;
    while (1)
    {
        // Attendre un signal dans la file
        if (xQueueReceive(erase_wifi_queue, &signal, portMAX_DELAY))
        {
            _lock_acquire(&lvgl_api_lock);
            lv_obj_t *main_screen = lv_screen_active();
            lv_obj_t *tmp_screen = NULL;
            tmp_screen = lv_obj_create(NULL);
            if (!tmp_screen)
            {
                printf("Error while creating new screen");
                continue;
            }
            lv_obj_set_style_bg_color(tmp_screen, lv_color_hex(0x000000), LV_PART_MAIN);
            lv_screen_load_anim(tmp_screen, LV_SCR_LOAD_ANIM_NONE, 50, 0, false);

            lv_obj_t *label = NULL;
            label = lv_label_create(tmp_screen);
            if (!label)
            {
                printf("Error while creating new label");
                continue;
            }
            _lock_release(&lvgl_api_lock);

            lcd_display_text(display, label, "Erasing WiFi credentials...", &lv_font_montserrat_16, lv_color_hex(0xffffff), LV_ALIGN_CENTER);

            esp_err_t err = nvs_Storage_EraseWifiCreds();
            if (err == ESP_OK)
                lcd_display_text(display, label, "Successfully erased !", &lv_font_montserrat_16, lv_color_hex(0xffffff), LV_ALIGN_CENTER);

            else
                lcd_display_text(display, label, "An error happened", &lv_font_montserrat_16, lv_color_hex(0xffffff), LV_ALIGN_CENTER);

            vTaskDelay(2000 / portTICK_PERIOD_MS);

            _lock_acquire(&lvgl_api_lock);
            lv_screen_load_anim(main_screen, LV_SCR_LOAD_ANIM_NONE, 50, 0, true);
            _lock_release(&lvgl_api_lock);
        }
    }
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
    lv_obj_t *main_label = NULL;

    lvgl_api_lock = get_lvgl_api_lock();

    main_label = lcd_display_text(display, NULL, "Connecting to internet...", &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_CENTER);

    // Connecting to an AP
    esp_netif_t *sta_handle = wifi_init_sta();
    if (sta_handle == NULL)
    {
        wifi_init_ap();
        httpServer_start(save_credentials_from_http);
        DNSserver *server = DNSserver_StartSocket();

        lcd_display_text(display, main_label, "Please connect to the WiFi :", &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_CENTER);
        _lock_acquire(&lvgl_api_lock);

        lv_obj_t *under_label = NULL;
        under_label = lv_label_create(main_label);
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

                lcd_display_text(display, main_label, "The smart clock will reboot in 3 seconds...", &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_CENTER);

                vTaskDelay(3000 / portTICK_PERIOD_MS);
                esp_restart();
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    // Setting up the push button and its interrupt

    // Timer to debounce the push button
    debounce_timer = xTimerCreate(
        "DebounceTimer",
        pdMS_TO_TICKS(300),
        pdFALSE,
        0,
        debounce_timer_cb);

    erase_wifi_queue = xQueueCreate(10, sizeof(uint8_t));
    if (erase_wifi_queue == NULL)
    {
        ESP_LOGE("MAIN", "Error while creating queue for the push button");
        return;
    }

    gpio_config_t config = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_14),
        .pull_up_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&config);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_14, button_isr_handler, NULL);

    xTaskCreate(erase_wifi_task, "Erase Creds Task", 2048, display, 8, NULL);

    // Display info text telling the user how to reset credentials
    lv_obj_t *msg_erase = NULL;
    msg_erase = lcd_display_text(display, msg_erase, "Press the button to erase wifi credentials", &lv_font_montserrat_16, lv_color_hex(0xffffff), LV_ALIGN_BOTTOM_MID);
    _lock_acquire(&lvgl_api_lock);
    lv_obj_set_style_pad_bottom(msg_erase, 16, LV_PART_MAIN);
    _lock_release(&lvgl_api_lock);

    initialize_sntp();
    // Displaying WiFi info
    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);

    char *buffer = NULL;
    buffer = malloc(64 * sizeof(char));

    xTaskCreate(display_time_task, "Clock task", 2048, display, 1, NULL);

    char ssid[33];
    memcpy(ssid, ap_info.ssid, sizeof(ssid));
    sprintf(buffer, "Connected to : %s", ssid);

    lcd_display_text(display, main_label, buffer, &lv_font_montserrat_20, lv_color_hex(0xffffff), LV_ALIGN_TOP_MID);

    _lock_acquire(&lvgl_api_lock);
    if (main_label)
        lv_obj_set_style_pad_top(main_label, 16, LV_PART_MAIN);
    else
        ESP_LOGE("MAIN", "label error");
    _lock_release(&lvgl_api_lock);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    _lock_acquire(&lvgl_api_lock);
    lv_obj_delete(main_label);
    _lock_release(&lvgl_api_lock);
}