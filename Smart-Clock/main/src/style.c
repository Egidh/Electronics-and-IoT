#include "style.h"

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
    _lock_t lvgl_api_lock = get_lvgl_api_lock();

    _lock_acquire(&lvgl_api_lock);
    lv_style_init(self);

    lv_style_set_text_color(self, lv_color_hex(0xffffff));
    lv_style_set_text_font(self, &lv_font_montserrat_14);
    lv_style_set_text_align(self, text_align);

    lv_style_set_align(self, align);
    _lock_release(&lvgl_api_lock);

    return self;
}