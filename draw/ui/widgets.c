//
// Created by wright on 10/3/26.
//

#include "widgets.h"

#include <ctype.h>

#include "../ui.impl.h"
#include "../../memory.h"
#include "../../debug.h"
#include "../../maths.h"
#include "../cmds/draw_text.impl.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSITIONTYPE i32

#include "../../../libs/stb_textedit.h"

typedef struct SINGLE_LINE_DATA_T {
    f32x2 row_size_px;
} SingleLineData;

struct WIDGET_TEXT_EDIT_HANDLE {
    STB_TexteditState stb_state;
    bool is_multi_line;

    union {
        SingleLineData single;
    } data;
};

typedef enum KEY_MODIFIER_E : i32 {
#define X_KEY_MODIFIERS \
    X(SHIFT) \
    X(CTRL) \
    X(ALT)
#define X(mod) \
    KEY_MODIFIER_##mod,
    X_KEY_MODIFIERS
#undef X
    KEY_MODIFIER_COUNT
} KeyModifier;

typedef enum KEY_MODIFIER_FLAG_E : u64 {
#define X(mod) \
    KEY_MODIFIER_##mod##_FLAG = 1ULL << (63 - KEY_MODIFIER_##mod),
    X_KEY_MODIFIERS
#undef X
} KeyModifierFlag;

#define STB_TEXTEDIT_KEYTYPE u64
#define STB_TEXTEDIT_K_SHIFT ((u64)KEY_MODIFIER_SHIFT_FLAG)
#define STB_TEXTEDIT_K_LEFT (KEYBOARD_KEY_ARROW_LEFT)
#define STB_TEXTEDIT_K_RIGHT (KEYBOARD_KEY_ARROW_RIGHT)
#define STB_TEXTEDIT_K_UP (KEYBOARD_KEY_ARROW_UP)
#define STB_TEXTEDIT_K_DOWN (KEYBOARD_KEY_ARROW_DOWN)
#define STB_TEXTEDIT_K_PGUP (KEYBOARD_KEY_PAGE_UP)
#define STB_TEXTEDIT_K_PGDOWN (KEYBOARD_KEY_PAGE_DOWN)
#define STB_TEXTEDIT_K_LINESTART (KEYBOARD_KEY_HOME)
#define STB_TEXTEDIT_K_LINEEND (KEYBOARD_KEY_END)
#define STB_TEXTEDIT_K_TEXTSTART ((u64)KEY_MODIFIER_CTRL_FLAG | KEYBOARD_KEY_HOME)
#define STB_TEXTEDIT_K_TEXTEND ((u64)KEY_MODIFIER_CTRL_FLAG | KEYBOARD_KEY_END)
#define STB_TEXTEDIT_K_DELETE (KEYBOARD_KEY_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE (KEYBOARD_KEY_BACKSPACE)
#define STB_TEXTEDIT_K_UNDO ((u64)KEY_MODIFIER_CTRL_FLAG | KEYBOARD_KEY_Z)
#define STB_TEXTEDIT_K_REDO ((u64)KEY_MODIFIER_CTRL_FLAG | KEYBOARD_KEY_Y)

#define STB_TEXTEDIT_STRING WidgetTextEditString
#define STB_TEXTEDIT_STRINGLEN(obj) ((obj)->chars.len)
#define STB_TEXTEDIT_GETCHAR(obj, idx) ((obj)->chars.data[(idx)])
#define STB_TEXTEDIT_NEWLINE ((int)'\n')
#define STB_TEXTEDIT_DELETECHARS(obj, i, n) delete_chars((obj), (i), (n))
#define STB_TEXTEDIT_INSERTCHARS(obj, i, c, n) insert_chars((obj), (i), (c), (n))
#define STB_TEXTEDIT_KEYTOTEXT(k) key_to_text((k))
#define STB_TEXTEDIT_LAYOUTROW(r, obj, idx) layout_row((r), (obj), (idx))
#define STB_TEXTEDIT_GETWIDTH(obj, n, i) get_width((obj), (n), (i))

static void layout_row(StbTexteditRow *layout, WidgetTextEditString *text, i32 start_idx) {
    if (!text->handle->is_multi_line) {
        layout->num_chars = (i32) text->chars.len - start_idx;
        Clay_Dimensions pre_dim = ui_impl_clay_measure_text(
            (Clay_StringSlice){
                .baseChars = text->chars.data,
                .chars = text->chars.data,
                .length = start_idx,
            },
            &text->config,
            text->ui
        );

        layout->x0 = pre_dim.width;

        Clay_Dimensions post_dim = ui_impl_clay_measure_text(
            (Clay_StringSlice){
                .baseChars = text->chars.data,
                .chars = &text->chars.data[start_idx],
                .length = layout->num_chars,
            },
            &text->config,
            text->ui
        );

        layout->x1 = layout->x0 + post_dim.width;

        f32 row_height = MAX(pre_dim.height, post_dim.height);

        u16 font_theme_idx = text->config.fontId & 0xff;
        u16 font_mod_flags = text->config.fontId & ~font_theme_idx;

        f32x2 start_cursor = draw_text_impl_get_start_cursor(
            (string_view){
                .start = text->chars.data,
                .len = text->chars.len,
            },
            ui_impl_get_font_sets(text->ui, font_theme_idx),
            ui_impl_read_font_modifier_flags(font_mod_flags),
            text->config.fontSize,
            row_height
        );

        layout->ymin = start_cursor.y;
        layout->ymax = row_height - start_cursor.y;

        layout->baseline_y_delta = layout->ymin;
    } else {
        crash_msg("Multi-line layout not implemented\n");
    }
}

static float get_width(WidgetTextEditString *text, i32 start_idx, i32 current_idx) {
    float x_delta = 0;

    if (!text->handle->is_multi_line) {
        Clay_Dimensions dim = ui_impl_clay_measure_text(
            (Clay_StringSlice){
                .baseChars = text->chars.data,
                .chars = &text->chars.data[start_idx],
                .length = current_idx - start_idx,
            },
            &text->config,
            text->ui
        );

        x_delta = dim.width;
    } else {
        crash_msg("Multi-line layout not implemented\n");
    }

    return x_delta;
}

static int key_to_text(u64 key) {
    int codepoint = -1;

    bool shift_pressed = key & KEY_MODIFIER_SHIFT_FLAG;
    key &= ~(KEY_MODIFIER_SHIFT_FLAG | KEY_MODIFIER_CTRL_FLAG | KEY_MODIFIER_ALT_FLAG);

    switch ((KeyboardKey) key) {
        case KEYBOARD_KEY_A: {
            codepoint = 'a';
            break;
        }
        case KEYBOARD_KEY_B: {
            codepoint = 'b';
            break;
        }
        case KEYBOARD_KEY_C: {
            codepoint = 'c';
            break;
        }
        case KEYBOARD_KEY_D: {
            codepoint = 'd';
            break;
        }
        case KEYBOARD_KEY_E: {
            codepoint = 'e';
            break;
        }
        case KEYBOARD_KEY_F: {
            codepoint = 'f';
            break;
        }
        case KEYBOARD_KEY_G: {
            codepoint = 'g';
            break;
        }
        case KEYBOARD_KEY_H: {
            codepoint = 'h';
            break;
        }
        case KEYBOARD_KEY_I: {
            codepoint = 'i';
            break;
        }
        case KEYBOARD_KEY_J: {
            codepoint = 'j';
            break;
        }
        case KEYBOARD_KEY_K: {
            codepoint = 'k';
            break;
        }
        case KEYBOARD_KEY_L: {
            codepoint = 'l';
            break;
        }
        case KEYBOARD_KEY_M: {
            codepoint = 'm';
            break;
        }
        case KEYBOARD_KEY_N: {
            codepoint = 'n';
            break;
        }
        case KEYBOARD_KEY_O: {
            codepoint = 'o';
            break;
        }
        case KEYBOARD_KEY_P: {
            codepoint = 'p';
            break;
        }
        case KEYBOARD_KEY_Q: {
            codepoint = 'q';
            break;
        }
        case KEYBOARD_KEY_R: {
            codepoint = 'r';
            break;
        }
        case KEYBOARD_KEY_S: {
            codepoint = 's';
            break;
        }
        case KEYBOARD_KEY_T: {
            codepoint = 't';
            break;
        }
        case KEYBOARD_KEY_U: {
            codepoint = 'u';
            break;
        }
        case KEYBOARD_KEY_V: {
            codepoint = 'v';
            break;
        }
        case KEYBOARD_KEY_W: {
            codepoint = 'w';
            break;
        }
        case KEYBOARD_KEY_X: {
            codepoint = 'x';
            break;
        }
        case KEYBOARD_KEY_Y: {
            codepoint = 'y';
            break;
        }
        case KEYBOARD_KEY_Z: {
            codepoint = 'z';
            break;
        }
        case KEYBOARD_KEY_NUM_0: {
            if (shift_pressed) {
                codepoint = ')';
            } else {
                codepoint = '0';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_1: {
            if (shift_pressed) {
                codepoint = '!';
            } else {
                codepoint = '1';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_2: {
            if (shift_pressed) {
                codepoint = '@';
            } else {
                codepoint = '2';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_3: {
            if (shift_pressed) {
                codepoint = '#';
            } else {
                codepoint = '3';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_4: {
            if (shift_pressed) {
                codepoint = '$';
            } else {
                codepoint = '4';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_5: {
            if (shift_pressed) {
                codepoint = '%';
            } else {
                codepoint = '5';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_6: {
            if (shift_pressed) {
                codepoint = '^';
            } else {
                codepoint = '6';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_7: {
            if (shift_pressed) {
                codepoint = '&';
            } else {
                codepoint = '7';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_8: {
            if (shift_pressed) {
                codepoint = '*';
            } else {
                codepoint = '8';
            }
            break;
        }
        case KEYBOARD_KEY_NUM_9: {
            if (shift_pressed) {
                codepoint = '(';
            } else {
                codepoint = '9';
            }
            break;
        }
        case KEYBOARD_KEY_MINUS: {
            if (shift_pressed) {
                codepoint = '_';
            } else {
                codepoint = '-';
            }
            break;
        }
        case KEYBOARD_KEY_EQUALS: {
            if (shift_pressed) {
                codepoint = '+';
            } else {
                codepoint = '=';
            }
            break;
        }
        case KEYBOARD_KEY_LEFT_BRACKET: {
            if (shift_pressed) {
                codepoint = '{';
            } else {
                codepoint = '[';
            }
            break;
        }
        case KEYBOARD_KEY_RIGHT_BRACKET: {
            if (shift_pressed) {
                codepoint = '}';
            } else {
                codepoint = ']';
            }
            break;
        }
        case KEYBOARD_KEY_BACKSLASH: {
            if (shift_pressed) {
                codepoint = '|';
            } else {
                codepoint = '\\';
            }
            break;
        }
        case KEYBOARD_KEY_SEMICOLON: {
            if (shift_pressed) {
                codepoint = ':';
            } else {
                codepoint = ';';
            }
            break;
        }
        case KEYBOARD_KEY_APOSTROPHE: {
            if (shift_pressed) {
                codepoint = '"';
            } else {
                codepoint = '\'';
            }
            break;
        }
        case KEYBOARD_KEY_BACKTICK: {
            if (shift_pressed) {
                codepoint = '~';
            } else {
                codepoint = '`';
            }
            break;
        }
        case KEYBOARD_KEY_COMMA: {
            if (shift_pressed) {
                codepoint = '<';
            } else {
                codepoint = ',';
            }
            break;
        }
        case KEYBOARD_KEY_PERIOD: {
            if (shift_pressed) {
                codepoint = '>';
            } else {
                codepoint = '.';
            }
            break;
        }
        case KEYBOARD_KEY_SLASH: {
            if (shift_pressed) {
                codepoint = '?';
            } else {
                codepoint = '/';
            }
            break;
        }
        case KEYBOARD_KEY_SPACE: {
            codepoint = ' ';
            break;
        }
        case KEYBOARD_KEY_ENTER: {
            codepoint = '\n';
            break;
        }
        default:
            break;
    }

    if (codepoint >= 0 && shift_pressed) {
        if (isalpha(codepoint)) {
            codepoint = toupper(codepoint);
        }
    }

    return codepoint;
}

static void delete_chars(WidgetTextEditString *text, i32 start_idx, i32 num_chars) {
    string_del(&text->chars, start_idx, num_chars);
}

static int insert_chars(WidgetTextEditString *text, i32 start_idx, const char *chars, i32 chars_len) {
    string_put(&text->chars, start_idx, "%.*s", chars_len, chars);
    return true;
}

#define STB_TEXTEDIT_IMPLEMENTATION
#include "../../../libs/stb_textedit.h"

WidgetTextEditHandle *widget_text_edit_create(bool multi_line) {
    WidgetTextEditHandle *handle = alt_malloc(sizeof(WidgetTextEditHandle));
    *handle = (WidgetTextEditHandle){
        .is_multi_line = multi_line,
    };

    stb_textedit_initialize_state(&handle->stb_state, !multi_line);

    return handle;
}

void widget_text_edit_destroy(WidgetTextEditHandle *handle) {
    alt_free(handle);
}

void widget_text_edit_ui(WidgetTextEditUiInfo *info) {
    STB_TexteditState *edit_state = &info->text->handle->stb_state;

    if (!info->text->handle->is_multi_line) {
        Clay_ElementData parent_data = Clay_GetElementData(info->parent_id);
        if (!parent_data.found) {
            crash_msg("Parent ID %.*s invalid\n", info->parent_id.stringId.length, info->parent_id.stringId.chars);
        }

        Clay_String clay_edit_str = {
            .isStaticallyAllocated = false,
            .chars = info->text->chars.data,
            .length = (i32) info->text->chars.len,
        };

        //@formatter:off
        CLAY_TEXT(clay_edit_str, &info->text->config);

        Clay_Dimensions post_dim = ui_impl_clay_measure_text(
            (Clay_StringSlice){
                .baseChars = info->text->chars.data,
                .chars = &info->text->chars.data[edit_state->cursor],
                .length = (i32)info->text->chars.len - edit_state->cursor, //edit_state->cursor,
            },
            &info->text->config,
            info->text->ui
        );

        CLAY({
            .backgroundColor = ui_color(info->cursor_color),
            .floating = {
                .attachPoints = {
                    .element = CLAY_ATTACH_POINT_RIGHT_TOP,
                    .parent = CLAY_ATTACH_POINT_RIGHT_TOP,
                },
                .attachTo = CLAY_ATTACH_TO_PARENT,
                .clipTo = CLAY_CLIP_TO_ATTACHED_PARENT,
                .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                .offset = {
                    .x = -post_dim.width,
                }
            },
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_FIXED(parent_data.boundingBox.height),
                    .width = CLAY_SIZING_FIXED(2),
                }
            }
        }){}
        //@formatter:on

        //Clay_ScrollContainerData parent_scroll = Clay_GetScrollContainerData(info->parent_id);
    } else {
        crash_msg("Multi-line text edit ui is not implemented yet\n");
    }
}

void widget_text_edit_click(WidgetTextEditString *text, f32x2 rel_pos) {
}

void widget_text_edit_drag(WidgetTextEditString *text, f32x2 rel_pos) {
}

i64 widget_text_edit_cut(WidgetTextEditString *text, string *out_str) {
    return 0;
}

i64 widget_text_edit_paste(WidgetTextEditString *text, const string *in_str) {
    return 0;
}

void widget_text_edit_key_press(WidgetTextEditString *text, WidgetTextEditKeyInput key_input) {
    u64 mod_key = key_input.key;

    if (key_input.mods.shift) {
        mod_key |= KEY_MODIFIER_SHIFT_FLAG;
    }

    if (key_input.mods.ctrl) {
        mod_key |= KEY_MODIFIER_CTRL_FLAG;
    }

    if (key_input.mods.alt) {
        mod_key |= KEY_MODIFIER_ALT_FLAG;
    }

    stb_textedit_key(text, &text->handle->stb_state, mod_key);
}
