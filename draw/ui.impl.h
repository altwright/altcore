//
// Created by wright on 10/3/26.
//

#ifndef BRITANNICUS_UI_IMPL_H
#define BRITANNICUS_UI_IMPL_H

#include "ui.h"

typedef struct UI_IMPL_MEASURE_TEXT_LINE_PARAMS_T {
    struct {
        bool left;
        bool right;
    } include_negative_side_bearings;
} UiImplMeasureTextLineParams;

Clay_Dimensions ui_impl_clay_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data);

FontSets *ui_impl_get_font_sets(UiContext *ui, i32 font_theme_idx);

FontStyle ui_impl_read_font_modifier_flags(u16 font_mod_flags);

UiImplMeasureTextLineParams ui_impl_get_measure_text_line_params(UiContext *ui);

void ui_impl_set_measure_text_line_params(UiContext* ui, const UiImplMeasureTextLineParams *params);

Clay_ElementId ui_impl_get_open_elem_id(UiContext *ui);

#endif //BRITANNICUS_UI_IMPL_H
