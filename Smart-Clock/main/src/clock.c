#include "clock.h"

void initialize_sntp(void)
{
    ESP_LOGI("MAIN", "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org"); // Serveur NTP par défaut
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Europe/Paris (été/hiver automatique)
    tzset();

    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI("MAIN", "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year >= (2020 - 1900))
    {
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI("MAIN", "The current date/time is: %s", strftime_buf);
    }
    else
    {
        ESP_LOGE("MAIN", "Failed to obtain time from NTP server");
    }
}

void display_time_task(void *pvParameter)
{
    lv_display_t *display = (lv_display_t *)pvParameter;
    char time_str[64];
    time_t now;
    struct tm timeinfo;
    lv_obj_t *time_label = NULL;

    while (1)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo); // Format HH:MM

        time_label = lcd_display_text(display, time_label, time_str, &lv_font_montserrat_20, lv_color_white(), LV_ALIGN_CENTER);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Mise à jour toutes les secondes
    }
}