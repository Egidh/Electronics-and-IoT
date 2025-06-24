/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --font DejaVuSansMono-wifi-ramp.ttf --symbols  --format lvgl -o WifiFont.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef WIFIFONT
#define WIFIFONT 1
#endif

#if WIFIFONT

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+E0D5 "" */
    0xf, 0xf0, 0x38, 0x1c, 0xc0, 0x3, 0x60, 0x6,
    0x30, 0xc, 0x18, 0x18, 0x8, 0x10, 0xc, 0x30,
    0x6, 0x60, 0x3, 0xc0, 0x1, 0x80, 0x1, 0x0,

    /* U+E0D6 "" */
    0xf, 0xf0, 0x38, 0x1c, 0xc0, 0x3, 0x60, 0x6,
    0x30, 0xc, 0x18, 0x18, 0x9, 0x90, 0xf, 0xf0,
    0x7, 0xe0, 0x3, 0xc0, 0x1, 0x80, 0x1, 0x0,

    /* U+E0D7 "" */
    0xf, 0xf0, 0x38, 0x1c, 0xc0, 0x3, 0x60, 0x6,
    0x33, 0x8c, 0x1f, 0xf8, 0xf, 0xf0, 0xf, 0xf0,
    0x7, 0xe0, 0x3, 0xc0, 0x1, 0x80, 0x1, 0x0,

    /* U+E0D8 "" */
    0xf, 0xf0, 0x38, 0x1c, 0xc0, 0x3, 0x67, 0xe6,
    0x3f, 0xfc, 0x1f, 0xf8, 0xf, 0xf0, 0xf, 0xf0,
    0x7, 0xe0, 0x3, 0xc0, 0x1, 0x80, 0x1, 0x0,

    /* U+E0D9 "" */
    0xf, 0xf0, 0x3f, 0xfc, 0xff, 0xff, 0x7f, 0xfe,
    0x3f, 0xfc, 0x1f, 0xf8, 0x1f, 0xf8, 0xf, 0xf0,
    0x7, 0xe0, 0x3, 0xc0, 0x1, 0x80, 0x1, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 257, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 24, .adv_w = 257, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 48, .adv_w = 257, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 72, .adv_w = 257, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 96, .adv_w = 257, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 57557, .range_length = 5, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};

extern const lv_font_t lv_font_wifi;


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t wifi_font = {
#else
lv_font_t WifiFont = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = -1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if WIFIFONT*/

