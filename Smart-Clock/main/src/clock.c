#include "clock.h"

void initialize_sntp(void)
{
    ESP_LOGI("MAIN", "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org"); // Default NTP server
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Europe/Paris (summer/winter)
    tzset();

    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI("MAIN", "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year < (2020 - 1900))
    {
        ESP_LOGE("MAIN", "Failed to obtain time from NTP server");
    }
}

void display_time_task(void *pvParameter)
{
    char time_str[16];
    char date_str[64];

    time_t now;
    struct tm timeinfo;

    lv_obj_t *time_label = NULL;
    lv_obj_t *date_label = NULL;

    _lock_t lvgl_api_lock = get_lvgl_api_lock();

    lv_style_t *time_style = get_big_label_default_style(LV_ALIGN_CENTER, LV_TEXT_ALIGN_CENTER);
    lv_style_t *date_style = get_mid_label_default_style(LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_LEFT);

    _lock_acquire(&lvgl_api_lock);
    lv_style_set_pad_left(date_style, 16);
    lv_style_set_pad_top(date_style, 16);
    _lock_release(&lvgl_api_lock);

    while (1)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo); // HH:MM format
        strftime(date_str, sizeof(date_str), "%a. %d %b", &timeinfo);

        time_label = ui_display_text(time_label, time_str, time_style);
        date_label = ui_display_text(date_label, date_str, date_style);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}