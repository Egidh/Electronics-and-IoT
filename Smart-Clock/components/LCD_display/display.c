#include "display.h"

esp_lcd_panel_io_handle_t panel_io_handle;
esp_lcd_panel_handle_t panel_handle;

static _lock_t lvgl_api_lock;

esp_err_t st7789_init()
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

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    return ESP_OK;
}

void lv_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    int32_t x_start = area->x1;
    int32_t x_end = area->x2;
    int32_t y_start = area->y1;
    int32_t y_end = area->y2;

    esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end + 1, y_end + 1, px_map);

    lv_display_flush_ready(display);
}

void lv_timer_task(void *pvParameter)
{
    while (true)
    {
        _lock_acquire(&lvgl_api_lock);
        uint32_t time_till_next = lv_timer_handler();
        _lock_release(&lvgl_api_lock);

        if (time_till_next == LV_NO_TIMER_READY)
            time_till_next = LV_DEF_REFR_PERIOD;

        vTaskDelay(time_till_next / portTICK_PERIOD_MS);
    }
}

lv_display_t *lcd_init()
{
    esp_lcd_panel_io_handle_t panel_io_handle;
    esp_lcd_panel_handle_t panel_handle;

    ESP_LOGI(TAG_LCD, "Initializing st7789 driver");
    st7789_init(&panel_io_handle, &panel_handle);

    ESP_LOGI(TAG_LCD, "Initializing LVGL display...");
    lv_init();

    lv_display_t *display = NULL;
    display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    assert(display && "Failed to create lvgl display");

    size_t buffer_size = LVGL_DRAW_BUF_LINES * LCD_WIDTH * sizeof(lv_color16_t);

    void *buf1 = spi_bus_dma_memory_alloc(LCD_SPI_HOST, buffer_size, 0);
    assert(buf1);
    void *buf2 = spi_bus_dma_memory_alloc(LCD_SPI_HOST, buffer_size, 0);
    assert(buf2);

    lv_display_set_buffers(display, buf1, buf2, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, lv_flush_cb);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);

    _lock_init(&lvgl_api_lock);

    lv_tick_set_cb(xTaskGetTickCount);
    xTaskCreate(lv_timer_task, "Timer task", 4096, NULL, 5, NULL);

    _lock_acquire(&lvgl_api_lock);
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_radius(scr, 20, LV_PART_MAIN);
    lv_obj_set_style_border_color(scr, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, 3, LV_PART_MAIN);
    _lock_release(&lvgl_api_lock);

    ESP_LOGI(TAG_LCD, "Succesfully initialized LCD display");

    return display;
}

lv_obj_t *lcd_display_text(lv_display_t *self, lv_obj_t *label, const char *text, const lv_font_t *font, lv_color_t color, lv_align_t align)
{
    if (!self)
    {
        ESP_LOGW(TAG_LCD, "Error displaying text: display is NULL");
        return NULL;
    }

    _lock_acquire(&lvgl_api_lock);
    lv_obj_t *screen = lv_display_get_screen_active(self);

    if (!label)
        label = lv_label_create(screen);

    lv_obj_set_align(label, align);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_width(label, LCD_WIDTH - 40);

    lv_label_set_text_static(label, text);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);

    _lock_release(&lvgl_api_lock);

    return label;
}

_lock_t get_lvgl_api_lock()
{
    return lvgl_api_lock;
}