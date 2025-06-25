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

void myclock_get_date(char *date_str, size_t size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(date_str, size, "%a. %d %b", &timeinfo);
}

void myclock_get_time(char *time_str, size_t size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(time_str, size, "%H:%M", &timeinfo);
}

