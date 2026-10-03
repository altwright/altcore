//
// Created by wright on 10/3/26.
//

#include "widgets.h"

#include "../ui.impl.h"
#include "../../memory.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSITIONTYPE i32

#include "../../../libs/stb_textedit.h"

struct WIDGET_TEXT_EDIT_HANDLE {
    STB_TexteditState stb_state;
    i64 edit_str_codepoint_count;
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

#define STB_TEXTEDIT_STRING WidgetTextEditInfo
#define STB_TEXTEDIT_STRINGLEN(obj) ((obj)->edit_str->len)
#define STB_TEXTEDIT_GETCHAR(obj, idx) ((obj)->edit_str->data[(idx)])
#define STB_TEXTEDIT_NEWLINE ((int)'\n')
#define STB_TEXTEDIT_DELETECHARS(obj, i, n) delete_chars((obj), (i), (n))
#define STB_TEXTEDIT_INSERTCHARS(obj, i, c, n) insert_chars((obj), (i), (c), (n))
#define STB_TEXTEDIT_KEYTOTEXT(k) key_to_text((k))
#define STB_TEXTEDIT_LAYOUTROW(r, obj, idx) layout_row((r), (obj), (idx))
#define STB_TEXTEDIT_GETWIDTH(obj, n, i) get_width((obj), (n), (i))

static void layout_row(StbTexteditRow *layout, WidgetTextEditInfo *info, i32 char_idx) {
}

static float get_width(WidgetTextEditInfo *info, i32 start_idx, i32 current_idx) {
    return 0;
}

static int key_to_text(u64 key) {
    return -1;
}

static void delete_chars(WidgetTextEditInfo *info, i32 start_idx, i32 num_chars) {

}

static int insert_chars(WidgetTextEditInfo *info, i32 start_idx, const char* chars, i32 chars_len) {
    return 0;
}

#define STB_TEXTEDIT_IMPLEMENTATION
#include "../../../libs/stb_textedit.h"

WidgetTextEditHandle *widget_text_edit_create(bool multi_line) {
    WidgetTextEditHandle *handle = alt_malloc(sizeof(WidgetTextEditHandle));
    *handle = (WidgetTextEditHandle){};

    stb_textedit_initialize_state(&handle->stb_state, !multi_line);

    return handle;
}

void widget_text_edit_destroy(WidgetTextEditHandle *handle) {
    alt_free(handle);
}
