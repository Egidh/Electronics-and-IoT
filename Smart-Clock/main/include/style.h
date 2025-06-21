#pragma once

#include "lvgl.h"
#include "display.h"

/// @brief Get the default style for big labels
/// @param align The label alignement
/// @param text_align The text alignement
/// @return The style
lv_style_t *get_big_label_default_style(lv_align_t align, lv_text_align_t text_align);

/// @brief Get the default style for mid labels
/// @param align The label alignement
/// @param text_align The text alignement
/// @return The style
lv_style_t *get_mid_label_default_style(lv_align_t align, lv_text_align_t text_align);

/// @brief Get the default style for little labels
/// @param align The label alignement
/// @param text_align The text alignement
/// @return The style
lv_style_t *get_little_label_default_style(lv_align_t align, lv_text_align_t text_align);
