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
    UI_FONT_MODIFIER_##mod##_FLAG = 1U << (UI_FONT_MODIFIER_##mod + 8),
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
    FontSet *data;
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

typedef enum UI_MOUSE_POINTER_ACTION_E : i32 {
#define X_UI_MOUSE_POINTER_ACTIONS \
    X(LEFT_CLICK) \
    X(RIGHT_CLICK) \
    X(MIDDLE_CLICK) \
    X(DOUBLE_CLICK)
#define X(action) \
    UI_MOUSE_POINTER_ACTION_##action,
    X_UI_MOUSE_POINTER_ACTIONS
#undef X
    UI_MOUSE_POINTER_ACTION_COUNT
} UiMousePointerAction;

typedef enum UI_MOUSE_POINTER_ACTION_FLAG_E : u64 {
#define X(action) \
    UI_MOUSE_POINTER_ACTION_##action##_FLAG = 1ULL << UI_MOUSE_POINTER_ACTION_##action,
    X_UI_MOUSE_POINTER_ACTIONS
#undef X
} UiMousePointerActionFlag;

typedef UiMousePointerActionFlag UiMousePointerActionFlags;

// Mouse activity should be reflected per-frame
typedef struct UI_MOUSE_INFO_T {
    struct {
        struct {
            f32x2 prev_frame;
            f32x2 curr_frame;
        } canvas_pos;

        struct {
            UiMousePointerActionFlags prev_frame;
            UiMousePointerActionFlags curr_frame;
        } pressed;
    } pointer;

    struct {
        f32x2 frame_delta_units;
        f64 height_pct_per_delta_unit;
        bool drag_scrolling;
    } scroll;

    struct {
        u64 last_left_click_time_ns;
    } trackers;
} UiMouseInfo;

typedef struct UI_BEGIN_LAYOUT_INFO_T {
    Framebuffer *canvas;
    UiMouseInfo mouse;
    f64 frame_elapsed_time_s;
} UiBeginLayoutInfo;

UiContext *ui_create(const UiCreateInfo *create_info);

void ui_destroy(UiContext *ui);

void ui_begin_layout(UiContext *ui, const UiBeginLayoutInfo *layout_info);

RenderCmds ui_end_layout(Arena *arena, UiContext *ui);

Clay_Color ui_color(rgba8 color);

void ui_set_strings(UiContext *ui, const char8_t **str_loc_sets, i64 num_str_loc_sets, i64 num_locs_per_set);

Clay_String ui_get_string(UiContext *ui, i64 str_idx);

void ui_set_locale(UiContext *ui, i64 loc_idx);

i64 ui_get_locale(UiContext *ui);

u16 ui_width_px(UiContext *ui, f64 pct);

u16 ui_height_px(UiContext *ui, f64 pct);

f32x2 ui_viewport_coord(UiContext *ui, f32x2 canvas_coord);

bool ui_elem_double_click(UiMouseInfo *info, bool is_pressed, bool this_frame);

bool ui_elem_left_button(UiMouseInfo *info, bool is_pressed, bool this_frame);

bool ui_elem_middle_button(UiMouseInfo *info, bool is_pressed, bool this_frame);

bool ui_elem_right_button(UiMouseInfo *info, bool is_pressed, bool this_frame);

bool ui_get_debug_enabled(UiContext *ui);

void ui_set_debug_enabled(UiContext *ui, bool enabled);

#endif //ALTCORE_UI_H
