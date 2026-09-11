//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_UI_H
#define ALTCORE_UI_H

#include "clay.h"
#include "renderer.h"

typedef struct UI_CONTEXT_T UiContext;

typedef struct UI_CREATE_INFO_T {
    i64 memory_cap;
    Clay_ErrorHandler err_handler;
    const Framebuffer *initial_canvas;
} UiCreateInfo;

typedef struct UI_BEGIN_LAYOUT_INFO_T {
    Framebuffer *canvas;
    f32x2 pointer_pos;
    bool pointer_pressed;
    f32x2 scroll_delta;
    f32 frame_elapsed_time_s;
} UiBeginLayoutInfo;

void ui_set_fonts(FontHandle **fonts, i64 fonts_len);

UiContext *ui_create(const UiCreateInfo *create_info);

void ui_destroy(UiContext *ui);

void ui_begin_layout(UiContext *ui, const UiBeginLayoutInfo *layout_info);

RenderCmds ui_end_layout(Arena *arena, UiContext *ui);

Clay_Color ui_color(RGBA8888 color);

Clay_Padding ui_padding(UiContext *ui, f32x4 padding_pct);

Clay_Padding ui_padding_all(UiContext *ui, f32 padding_pct);

#endif //ALTCORE_UI_H
