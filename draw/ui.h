//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_UI_H
#define ALTCORE_UI_H

#include <uchar.h>

#include "clay.h"
#include "renderer.h"

typedef struct UI_CONTEXT_T UiContext;

typedef enum UI_FONT_MODIFIER_E : i32 {
#define X_UI_FONT_MODIFIERS \
    X(ITALIC) \
    X(BOLD)
#define X(mod) \
    UI_FONT_MODIFIER_##mod,
    X_UI_FONT_MODIFIERS
#undef X
    UI_FONT_MODIFIER_COUNT
} UiFontModifier;

typedef enum UI_FONT_MODIFIER_FLAGS_E : u16 {
#define X(mod) \
    UI_FONT_MODIFIER_FLAG_##mod = 1U << (UI_FONT_MODIFIER_##mod + 8),
    X_UI_FONT_MODIFIERS
#undef X
} UiFontModifierFlags;


/*
 * Each font theme is an array of font sets in priority order
 * from highest priority to lowest priority in usage. If a
 * codepoint is not found in the a font set, the next font set in
 * terms of priority is searched.
 */
typedef struct UI_FONT_THEME_T {
    FontSet* data;
    i64 len;
} UiFontSets;

typedef struct UI_CREATE_INFO_T {
    i64 memory_cap;
    const Framebuffer *initial_canvas;
    f32 viewport_aspect_ratio;

    /*
     * A font theme can represent the sets of fonts used
     * for text sections such as Titles, Subtitles, Heading 1,
     * Heading 2, Body, etc.. These themes are defined user-side
     * and are indexed through the lower 8-bits of the fontId
     * parameter of CLAY_TEXT.
     */
    const struct {
        UiFontSets *data;
        /*
         * The fontId parameter of CLAY_TEXT permits only a maximum of 255 font themes to be indexed
         * using the lower 8-bits, with the upper 8-bits used for modifier flags.
         */
        u8 len;
    } font_themes;
} UiCreateInfo;

typedef struct UI_BEGIN_LAYOUT_INFO_T {
    Framebuffer *canvas;
    f32x2 pointer_pos;
    bool pointer_pressed;
    f32x2 scroll_delta;
    f32 frame_elapsed_time_s;
} UiBeginLayoutInfo;

UiContext *ui_create(const UiCreateInfo *create_info);

void ui_destroy(UiContext *ui);

void ui_begin_layout(UiContext *ui, const UiBeginLayoutInfo *layout_info);

RenderCmds ui_end_layout(Arena *arena, UiContext *ui);

Clay_Color ui_color(rgba8 color);

void ui_set_string(UiContext *ui, u64 str_key, u64 loc_key, const char8_t *utf8_str);

Clay_String ui_get_string(UiContext *ui, u64 str_key);

void ui_set_locale(UiContext *ui, u64 loc_key);

u64 ui_get_locale(UiContext *ui);

u16 ui_px_width(UiContext *ui, f32 pct);

u16 ui_px_height(UiContext *ui, f32 pct);

#endif //ALTCORE_UI_H
