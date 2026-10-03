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

typedef struct WIDGET_TEXT_EDIT_INFO_T {
    WidgetTextEditHandle *handle;
    UiContext *ui;
    Clay_TextElementConfig text_config;
    string *edit_str;
    bool edit_str_cap_fixed;
    Clay_String placeholder_str;
} WidgetTextEditInfo;

typedef struct WIDGET_TEXT_EDIT_KEY_INPUT_T {
    KeyboardKey key;

    struct {
        bool shift;
        bool ctrl;
        bool alt;
    } mods;
} WidgetTextEditKeyInput;

WidgetTextEditHandle *widget_text_edit_create(bool multi_line);

void widget_text_edit_destroy(WidgetTextEditHandle *handle);

void widget_text_edit_ui(WidgetTextEditInfo *info);

void widget_text_edit_click(WidgetTextEditInfo *info, f32x2 rel_pos);

void widget_text_edit_drag(WidgetTextEditInfo *info, f32x2 rel_pos);

i64 widget_text_edit_cut(WidgetTextEditInfo *info, string *out_str);

i64 widget_text_edit_paste(WidgetTextEditInfo *info, const string *in_str);

void widget_text_edit_key_action(WidgetTextEditInfo *info, WidgetTextEditKeyInput key_input);

#endif //BRITANNICUS_WIDGETS_H
