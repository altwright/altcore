//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_UI_H
#define ALTCORE_UI_H

#include <uchar.h>

#include "clay.h"
#include "renderer.h"

typedef struct UI_CONTEXT_T UiContext;

typedef struct UI_CREATE_INFO_T {
    i64 memory_cap;
    const Framebuffer *initial_canvas;

    // The font handle lifetimes must be managed externally
    struct {
        FontHandle **data;
        i64 len;
    } fonts;
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

Clay_Color ui_color(RGBA8888 color);

void ui_set_string(UiContext *ui, u64 str_key, u64 loc_key, const char8_t *utf8_str);

Clay_String ui_get_string(UiContext *ui, u64 str_key);

void ui_set_locale(UiContext *ui, u64 loc_key);

u64 ui_get_locale(UiContext *ui);

u16 ui_px_width(UiContext *ui, f32 pct);

u16 ui_px_height(UiContext *ui, f32 pct);

#endif //ALTCORE_UI_H
