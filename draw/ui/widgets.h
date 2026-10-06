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
    UiContext *ui;
    Clay_TextElementConfig config;
} WidgetTextEditString;

typedef struct WIDGET_TEXT_EDIT_INFO_T {
    WidgetTextEditString *text;
    rgba8 selection_color;
    rgba8 cursor_color;
    u16 cursor_width_px;
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

#endif //BRITANNICUS_WIDGETS_H
