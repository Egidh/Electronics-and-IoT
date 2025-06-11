#include "st7789.h"

esp_lcd_panel_io_handle_t panel_io_handle;
esp_lcd_panel_handle_t panel_handle;

void lcd_draw_rect(int x, int y, int width, int height, uint16_t color)
{
    uint16_t *pixels = heap_caps_malloc(sizeof(uint16_t) * width * height, MALLOC_CAP_DMA);
    if (!pixels)
    {
        ESP_LOGE(TAG_LCD, "Error while allocating pixels buffer");
        return;
    }
    for (int i = 0; i < width * height; i++)
        pixels[i] = color;

    esp_err_t err = esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + width, y + height, pixels);
    if (err != ESP_OK)
        ESP_LOGE(TAG_LCD, "Error while drawin rect : %s", esp_err_to_name(err));

    free(pixels);
}

esp_err_t lcd_init()
{
    spi_bus_config_t bus_config = {
        .max_transfer_sz = LCD_WIDTH * 80 * sizeof(uint16_t),
        .mosi_io_num = PIN_LCD_MOSI,
        .sclk_io_num = PIN_LCD_SCLK,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_LCD_CS,
        .dc_gpio_num = PIN_LCD_DC,
        .pclk_hz = SPI_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &panel_io_handle));

    esp_lcd_panel_dev_config_t dev_config = {
        .bits_per_pixel = 16,
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io_handle, &dev_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));  // needed because of my controller (st7789v3) specification
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));       // horizontal orientation
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false)); // (0;0) will be on the top left corner
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 34));      // There is an offset of 34 pixels on the x axis

    lcd_draw_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, 0x0000); // cleaning screen and ensuring to not have noise

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    return ESP_OK;
}