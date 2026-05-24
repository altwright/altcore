//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_RENDERER_H
#define ALTCORE_RENDERER_H

#include "../types.h"
#include "framebuffer.h"
#include "window.h"
#include "fonts.h"

struct RENDERER_T;
typedef struct RENDERER_T Renderer;

typedef enum RENDERER_TYPE_E {
#ifndef X_RENDERER_TYPES
#define X_RENDERER_TYPES \
    X(SOFTWARE_SINGLE_THREAD) \
    X(SOFTWARE_MULTI_THREAD) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    RENDERER_TYPE_##type,
#endif
    X_RENDERER_TYPES
#undef X
} RendererType;

typedef struct RENDERER_CREATE_INFO_T {
    RendererType type;
} RendererCreateInfo;

typedef enum RENDER_CMD_TYPE_E {
#ifndef X_RENDER_CMD_TYPES
#define X_RENDER_CMD_TYPES \
    X(CLEAR) \
    X(DRAW_RECT) \
    X(BLIT) \
    X(DRAW_TEXT) \
    X(SCISSOR) \
    X(PRESENT) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    RENDER_CMD_TYPE_##type,
#endif
    X_RENDER_CMD_TYPES
#undef X
} RenderCmdType;

typedef enum RENDER_BUFFER_TYPE_E {
#ifndef X_RENDER_BUFFER_TYPES
#define X_RENDER_BUFFER_TYPES \
    X(FRAMEBUFFER) \
    X(SWAPCHAIN) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    RENDER_BUFFER_TYPE_##type,
#endif
    X_RENDER_BUFFER_TYPES
#undef X
} RenderBufferType;

typedef struct RENDER_CMD_CLEAR_T {
    Framebuffer *framebuffer;
    RGBA8888 color;
} RenderCmdClear;

typedef struct RECT_CORNER_RADII_T {
    f32 top_left_px, top_right_px, bottom_left_px, bottom_right_px;
} RectCornerRadii;

typedef struct RECT_BORDER_WIDTHS_T {
    f32 left_px, right_px, top_px, bottom_px;
} RectBorderWidths;

typedef struct RENDER_CMD_DRAW_RECT_T {
    Framebuffer *framebuffer;
    f32x4 dst;
    RGBA8888 bg_color;
    RectCornerRadii corner_radii;
    RGBA8888 border_color;
    RectBorderWidths border_widths;
} RenderCmdDrawRect;

typedef struct RENDER_CMD_BLIT_T {
    Framebuffer *framebuffer;
    f32x4 dst;
    RectCornerRadii dst_corner_radii;
    u8 *src_pixels;
    PixelFormat src_px_format;
    i32x2 src_size;
    i64 src_pitch_bytes;
} RenderCmdBlit;

typedef struct RENDER_CMD_DRAW_TEXT_T {
    Framebuffer *framebuffer;
    char *text;
    i64 text_len;
    f32x4 dst;
    FontHandle *font;
    RGBA8888 color;
    i32 font_height_px;
    i32 letter_spacing_px;
    i32 line_height_px;
} RenderCmdDrawText;

typedef struct RENDER_CMD_SCISSOR_T {
    Framebuffer* framebuffer;
    f32x4 region;
} RenderCmdScissor;

typedef struct RENDER_CMD_PRESENT_T {
    WindowHandle *window;
    Framebuffer *framebuffer;
} RenderCmdPresent;

typedef struct RENDER_CMD_T {
    RenderCmdType type;

    union {
        RenderCmdClear clear;
        RenderCmdDrawRect draw_rect;
        RenderCmdBlit blit;
        RenderCmdDrawText draw_text;
        RenderCmdScissor scissor;
        RenderCmdPresent present;
    } data;
} RenderCmd;

typedef struct RENDER_CMD_BUFFER_T {
    ARRAY_FIELDS(RenderCmd)
} RenderCmdBuffer;

Renderer *renderer_create(const RendererCreateInfo *create_info);

void renderer_execute_cmd_buf(Renderer *renderer, RenderCmdBuffer *cmd_buf);

void renderer_destroy(Renderer *renderer);

void renderer_wait_until_idle(Renderer *renderer);

#endif //ALTCORE_RENDERER_H
