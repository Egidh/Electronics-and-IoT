#include "display.h"

esp_err_t st7789_init(int width, esp_lcd_panel_handle_t *panel_handle)
{
    esp_lcd_panel_io_handle_t panel_io_handle;

    spi_bus_config_t bus_config = {
        .max_transfer_sz = width * 80 * sizeof(uint16_t),
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
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io_handle, &dev_config, panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(*panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(*panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(*panel_handle, true));  // needed because of my controller (st7789v3) specification
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(*panel_handle, true));       // horizontal orientation
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(*panel_handle, true, false)); // (0;0) will be on the top left corner
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(*panel_handle, 0, 34));      // There is an offset of 34 pixels on the x axis

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(*panel_handle, true));

    return ESP_OK;
}
