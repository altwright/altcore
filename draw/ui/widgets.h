//
// Created by wright on 10/3/26.
//

#ifndef BRITANNICUS_WIDGETS_H
#define BRITANNICUS_WIDGETS_H

#include "../ui.h"
#include "../../types.h"
#include "../../strings.h"
#include "../../events.h"

typedef struct WIDGET_TEXT_EDIT_HANDLE WidgetTextEditHandle;

typedef struct WIDGET_TEXT_EDIT_STRING_T {
    WidgetTextEditHandle *handle;
    string chars;
    bool cap_fixed;
    UiContext *ui;
    Clay_TextElementConfig config;
} WidgetTextEditString;

typedef struct WIDGET_TEXT_EDIT_UI_INFO_T {
    WidgetTextEditString *text;
    rgba8 selection_color;
    rgba8 cursor_color;
    u16 cursor_width_px;
    rgba8 insert_cursor_color;
    UiMouseInfo *mouse;
} WidgetTextEditUiInfo;

typedef struct WIDGET_TEXT_EDIT_KEY_INPUT_T {
    KeyboardKey key;

    struct {
        bool shift;
        bool ctrl;
        bool alt;
    } mods;
} WidgetTextEditKeyInput;

typedef struct WIDGET_I64_SCALE_UI_INFO_T {
    i64* value;
    i64 min, max;
    UiContext *ui;
    UiMouseInfo *mouse;
    bool vertical;
} WidgetI64ScaleUiInfo;

typedef struct WIDGET_F64_SCALE_UI_INFO_T {
    f64* value;
    f64 min, max;
    UiContext *ui;
    UiMouseInfo *mouse;
    bool vertical;
} WidgetF64ScaleUiInfo;

WidgetTextEditHandle *widget_text_edit_create();

void widget_text_edit_destroy(WidgetTextEditHandle *handle);

void widget_text_edit_ui(WidgetTextEditUiInfo *info);

void widget_text_edit_click(WidgetTextEditString *text, f32x2 rel_pos);

void widget_text_edit_drag(WidgetTextEditString *text, f32x2 rel_pos);

// Returns number of bytes in selection, if it exists
i64 widget_text_edit_cut(WidgetTextEditString *text, string *out_str);

// Returns number of bytes pasted into selection, if it exists
i64 widget_text_edit_paste(WidgetTextEditString *text, const string *in_str);

void widget_text_edit_key_press(WidgetTextEditString *text, WidgetTextEditKeyInput key_input);

void widget_i64_scale_ui(WidgetI64ScaleUiInfo *info);

void widget_f64_scale_ui(WidgetF64ScaleUiInfo *info);

#endif //BRITANNICUS_WIDGETS_H
