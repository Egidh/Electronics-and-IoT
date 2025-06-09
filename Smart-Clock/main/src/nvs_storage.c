#include "nvs_storage.h"

bool nvs_Storage_GetWifiCreds(char *ssid, char *passwd)
{
    nvs_handle_t nvs;
    esp_err_t err;
    err = nvs_open("creds", NVS_READONLY, &nvs);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG_NVS, "Could not open namespace creds, it might not exist yet");
        return false;
    }

    size_t element_size;

    // Getting SSID
    if (nvs_get_str(nvs, "ssid", NULL, &element_size) == ESP_OK)
    {
        err = nvs_get_str(nvs, "ssid", ssid, &element_size);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG_NVS, "ssid : %s\n", ssid);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG_NVS, "No data found\n");
            nvs_close(nvs);
            return false;
        case ESP_FAIL:
            ESP_LOGE(TAG_NVS, "Unknown error, reinitializing nvs might be a solution");
            nvs_close(nvs);
            return false;
        default:
            ESP_LOGE(TAG_NVS, "An error happened, no further information available");
            nvs_close(nvs);
            return false;
        }
    }
    else
    {
        nvs_close(nvs);
        return false;
    }

    // Getting password
    if (nvs_get_str(nvs, "passwd", NULL, &element_size) == ESP_OK)
    {
        esp_err_t err = nvs_get_str(nvs, "passwd", passwd, &element_size);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG_NVS, "password : %s\n", passwd);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG_NVS, "No data found\n");
            nvs_close(nvs);
            return false;
        case ESP_FAIL:
            ESP_LOGE(TAG_NVS, "Unknown error, reinitializing nvs might be a solution");
            nvs_close(nvs);
            return false;
        default:
            ESP_LOGE(TAG_NVS, "An error happened while reading, no further information available");
            nvs_close(nvs);
            return false;
        }
    }
    else
    {
        nvs_close(nvs);
        return false;
    }

    nvs_close(nvs);
    return true;
}

esp_err_t nvs_Storage_SetWifiCreds(char *ssid, char *passwd)
{
    if (strlen(ssid) >= 32)
    {
        ESP_LOGI(TAG_NVS, "Incorrect SSID length, must be at most 32 characters. Nothing has been written.");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(passwd) >= 64)
    {
        ESP_LOGI(TAG_NVS, "Incorrect password length, must be at most 64 characters. Nothing has been written.");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("creds", NVS_READWRITE, &nvs));

    esp_err_t err;

    // Writing ssid
    err = nvs_set_str(nvs, "ssid", ssid);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG_NVS, "ssid successfully written");
        break;

    case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
        ESP_LOGE(TAG_NVS, "Not enough space available");
        break;

    case ESP_ERR_NVS_REMOVE_FAILED:
        ESP_LOGE(TAG_NVS, "Failed to remove key during write operation");
        break;

    default:
        ESP_LOGE(TAG_NVS, "An error happened while writing, no further information available");
        break;
    }
    ESP_ERROR_CHECK(err);

    // Writing passwd
    err = nvs_set_str(nvs, "passwd", passwd);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG_NVS, "password successfully written");
        break;

    case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
        ESP_LOGE(TAG_NVS, "Not enough space available");
        break;

    case ESP_ERR_NVS_REMOVE_FAILED:
        ESP_LOGE(TAG_NVS, "Failed to remove key during write operation");
        break;

    default:
        ESP_LOGE(TAG_NVS, "An error happened while writing, no further information available");
        break;
    }
    ESP_ERROR_CHECK(err);

    nvs_close(nvs);
    return ESP_OK;
}

esp_err_t nvs_Storage_EraseWifiCreds()
{
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("creds", NVS_READWRITE, &nvs));

    esp_err_t err;

    err = nvs_erase_key(nvs, "ssid");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG_NVS, "Error erasing ssid: %s", esp_err_to_name(err));
        nvs_close(nvs);
        return err;
    }

    err = nvs_erase_key(nvs, "passwd");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG_NVS, "Error erasing password: %s", esp_err_to_name(err));
        nvs_close(nvs);
        return err;
    }

    err = nvs_commit(nvs);
    if (err != ESP_OK)
        ESP_LOGE(TAG_NVS, "Error while committing changes: %s", esp_err_to_name(err));
    else
        ESP_LOGI(TAG_NVS, "WiFi credentials erased successfully !");

    nvs_close(nvs);
    return err;
}