#include "lvgl_UI.h"

// Mutex for the lvgl api
_lock_t lvgl_api_lock = NULL;

_lock_t get_lvgl_api_lock()
{
    return lvgl_api_lock;
}

/*****************************************************************************
 *                                 Styles                                    *
 *****************************************************************************/

static lv_style_t *notif_style = NULL;

static lv_style_t *big_label_style = NULL;
static lv_style_t *mid_label_style = NULL;
static lv_style_t *little_label_style = NULL;

lv_style_t *get_big_label_default_style(lv_align_t align, lv_text_align_t text_align)
{
    lv_style_t *self = malloc(sizeof(lv_style_t));
    _lock_t lvgl_api_lock = get_lvgl_api_lock();

    _lock_acquire(&lvgl_api_lock);
    lv_style_init(self);

    lv_style_set_text_color(self, lv_color_hex(0xffffff));
    lv_style_set_text_font(self, &lv_font_montserrat_20);
    lv_style_set_text_align(self, text_align);

    lv_style_set_align(self, align);
    _lock_release(&lvgl_api_lock);

    return self;
}

lv_style_t *get_mid_label_default_style(lv_align_t align, lv_text_align_t text_align)
{
    lv_style_t *self = malloc(sizeof(lv_style_t));
    _lock_t lvgl_api_lock = get_lvgl_api_lock();

    _lock_acquire(&lvgl_api_lock);
    lv_style_init(self);

    lv_style_set_text_color(self, lv_color_hex(0xffffff));
    lv_style_set_text_font(self, &lv_font_montserrat_16);
    lv_style_set_text_align(self, text_align);

    lv_style_set_align(self, align);
    _lock_release(&lvgl_api_lock);

    return self;
}

lv_style_t *get_little_label_default_style(lv_align_t align, lv_text_align_t text_align)
{
    lv_style_t *self = malloc(sizeof(lv_style_t));

    _lock_acquire(&lvgl_api_lock);
    lv_style_init(self);

    lv_style_set_text_color(self, lv_color_hex(0xffffff));
    lv_style_set_text_font(self, &lv_font_montserrat_14);
    lv_style_set_text_align(self, text_align);

    lv_style_set_align(self, align);
    _lock_release(&lvgl_api_lock);

    return self;
}

static void init_notification_default_style()
{
    notif_style = malloc(sizeof(lv_style_t));

    _lock_acquire(&lvgl_api_lock);
    lv_style_init(notif_style);

    lv_style_set_bg_color(notif_style, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(notif_style, LV_OPA_100);

    lv_style_set_text_color(notif_style, lv_color_hex(0x000000));
    lv_style_set_text_font(notif_style, &lv_font_montserrat_16);
    lv_style_set_text_align(notif_style, LV_TEXT_ALIGN_CENTER);

    lv_style_set_radius(notif_style, 25);
    lv_style_set_pad_all(notif_style, 8);
    lv_style_set_align(notif_style, LV_ALIGN_TOP_MID);
    _lock_release(&lvgl_api_lock);
}

static void init_labels_default_styles()
{
    little_label_style = malloc(sizeof(lv_style_t));

    _lock_acquire(&lvgl_api_lock);

    // Little label
    little_label_style = malloc(sizeof(lv_style_t));
    lv_style_init(little_label_style);
    lv_style_set_text_color(little_label_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(little_label_style, &lv_font_montserrat_14);

    // Mid label
    mid_label_style = malloc(sizeof(lv_style_t));
    lv_style_init(mid_label_style);
    lv_style_set_text_color(mid_label_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(mid_label_style, &lv_font_montserrat_16);

    // Big label
    big_label_style = malloc(sizeof(lv_style_t));
    lv_style_init(big_label_style);
    lv_style_set_text_color(big_label_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(big_label_style, &lv_font_montserrat_20);

    _lock_release(&lvgl_api_lock);
}

lv_style_t *get_notification_default_style()
{
    return notif_style;
}

/*****************************************************************************
 *                               Animations                                  *
 *****************************************************************************/
static lv_anim_t notif_anim;

static void notification_delete_cb(lv_anim_t *anim)
{
    lv_obj_t *obj = (lv_obj_t *)anim->var;

    if (obj != NULL)
        lv_obj_delete_async(obj);
}

static void notif_anim_y_cb(void *obj, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)obj, v);
}

static void ui_init_notif_anim()
{
    _lock_acquire(&lvgl_api_lock);
    lv_anim_init(&notif_anim);
    lv_anim_set_path_cb(&notif_anim, lv_anim_path_ease_in_out);
    lv_anim_set_duration(&notif_anim, 500);
    lv_anim_set_reverse_duration(&notif_anim, 500);
    lv_anim_set_exec_cb(&notif_anim, notif_anim_y_cb);
    lv_anim_set_completed_cb(&notif_anim, notification_delete_cb);
    _lock_release(&lvgl_api_lock);
}

/*****************************************************************************
 *                                   UI                                      *
 *****************************************************************************/
esp_lcd_panel_handle_t panel_handle;

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

uint32_t tick_cb()
{
    return esp_timer_get_time() / 1000;
}

lv_display_t *ui_init()
{
    ESP_LOGI(TAG_LCD, "Initializing st7789 driver");
    st7789_init(LCD_WIDTH, &panel_handle);
    ESP_LOGI(TAG_LCD, "Succesfully initialized LCD display");

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

    lv_tick_set_cb(tick_cb);
    xTaskCreate(lv_timer_task, "Timer task", 5120, NULL, 5, NULL);

    _lock_acquire(&lvgl_api_lock);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);

    // Basic display
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_radius(scr, 25, LV_PART_MAIN);
    lv_obj_set_style_border_color(scr, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, UI_BORDER_SIZE, LV_PART_MAIN);
    _lock_release(&lvgl_api_lock);

    // Styles initialization
    init_labels_default_styles();
    init_notification_default_style();

    // Animations initialization
    ui_init_notif_anim();

    return display;
}

lv_obj_t *ui_create_wifi_object(const char *ssid, lv_align_t align, int padding_x, int padding_y)
{
    _lock_t lvgl_api_lock = get_lvgl_api_lock();

    // Creating the container for the WiFi info
    _lock_acquire(&lvgl_api_lock);
    lv_obj_t *wifi_container = lv_obj_create(lv_layer_top());
    lv_obj_set_size(wifi_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(wifi_container, align, 0, 0);
    lv_obj_set_style_bg_opa(wifi_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(wifi_container, LV_OPA_TRANSP, 0);

    lv_obj_set_layout(wifi_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(wifi_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(wifi_container, 6, 0);
    _lock_release(&lvgl_api_lock);

    // Labels for the ssid name and the icon for the strength of the WiFi connection
    lv_obj_t *wifi_label_text = NULL;
    lv_obj_t *wifi_label_icon = NULL;

    lv_style_t *wifi_style = get_mid_label_default_style(LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_RIGHT);

    // Setting up and displaying both labels
    _lock_acquire(&lvgl_api_lock);
    wifi_label_text = lv_label_create(wifi_container);
    wifi_label_icon = lv_label_create(wifi_container);

    lv_obj_set_style_border_opa(wifi_label_icon, LV_OPA_100, 0);
    lv_obj_set_style_border_opa(wifi_label_text, LV_OPA_100, 0);

    lv_obj_add_style(wifi_label_text, wifi_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(wifi_label_icon, &wifi_font, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifi_label_icon, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_label_set_text(wifi_label_text, ssid);
    lv_label_set_text(wifi_label_icon, ICON_WIFI_HIGH);
    _lock_release(&lvgl_api_lock);

    return wifi_label_icon;
}

lv_obj_t *ui_display_text(lv_obj_t *label, const char *text, const lv_style_t *style)
{
    if (!text)
        return NULL;

    _lock_acquire(&lvgl_api_lock);
    if (!label)
    {
        label = lv_label_create(lv_screen_active());
        lv_obj_add_style(label, style, LV_PART_MAIN);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_WRAP);
    }

    int32_t width = lv_text_get_width(text, strlen(text), lv_obj_get_style_text_font(label, LV_PART_MAIN), LV_STYLE_TEXT_LETTER_SPACE);
    if (width > LCD_WIDTH - 40)
        lv_obj_set_width(label, LCD_WIDTH - 40);

    lv_label_set_text(label, text);
    _lock_release(&lvgl_api_lock);

    return label;
}

void ui_send_notification(const char *text, uint32_t delay_ms)
{
    if (!text)
        return;

    _lock_acquire(&lvgl_api_lock);

    lv_obj_t *self = lv_label_create(lv_layer_top());
    lv_label_set_text(self, text);
    lv_label_set_long_mode(self, LV_LABEL_LONG_MODE_WRAP);

    lv_obj_add_style(self, notif_style, LV_PART_MAIN);
    lv_obj_set_width(self, LCD_WIDTH - 16);

    lv_anim_t anim = notif_anim;
    lv_obj_update_layout(self);
    lv_anim_set_values(&anim, -lv_obj_get_height(self), 2 * UI_BORDER_SIZE);
    lv_anim_set_var(&anim, self);
    lv_anim_set_reverse_delay(&anim, delay_ms);

    lv_anim_start(&anim);

    _lock_release(&lvgl_api_lock);
}

lv_obj_t *ui_message_box_create(const char *title, const char *msg)
{
    if (!title || !msg)
        return NULL;

    lv_obj_t *self = NULL;

    _lock_acquire(&lvgl_api_lock);
    self = lv_obj_create(lv_screen_active());

    lv_obj_set_size(self, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(self, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(self, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(self, LV_OPA_100, 0);
    lv_obj_set_style_border_color(self, lv_color_hex3(0xfff), LV_PART_MAIN);
    lv_obj_set_style_pad_all(self, 0, LV_PART_MAIN);

    lv_obj_set_layout(self, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(self, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(self, 6, 0);

    lv_obj_t *header = lv_label_create(self);
    lv_obj_t *content = lv_label_create(self);

    lv_obj_add_style(header, mid_label_style, LV_PART_MAIN);
    lv_obj_add_style(content, mid_label_style, LV_PART_MAIN);

    lv_obj_set_style_bg_color(header, lv_color_hex3(0xfff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 8, LV_PART_MAIN);
    lv_obj_set_style_text_color(header, lv_color_hex3(0x000), LV_PART_MAIN);

    lv_obj_set_style_pad_left(content, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 6, LV_PART_MAIN);

    lv_label_set_text(header, title);
    lv_label_set_text(content, msg);

    lv_label_set_long_mode(content, LV_LABEL_LONG_MODE_WRAP);

    _lock_release(&lvgl_api_lock);

    return self;
}

void ui_delete_obj(lv_obj_t *self)
{
    if (!self)
        return;

    _lock_acquire(&lvgl_api_lock);
    lv_obj_delete(self);
    _lock_release(&lvgl_api_lock);

    self = NULL;
}
