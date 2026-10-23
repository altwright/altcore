//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_RENDERER_H
#define ALTCORE_RENDERER_H

#include "../types.h"
#include "framebuffer.h"
#include "model.h"
#include "shader.h"
#include "window.h"
#include "fonts.h"

typedef struct RENDERER_T Renderer;

typedef enum RENDERER_TYPE_E {
#ifndef X_RENDERER_TYPES
#define X_RENDERER_TYPES \
    X(SINGLE_THREAD) \
    X(MULTI_THREAD) \
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
    X(DRAW_RECT) \
    X(DRAW_TEXT) \
    X(SCISSOR) \
    X(DRAW_MODEL) \
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

typedef struct RENDER_CMD_DRAW_RECT_CORNER_RADII_T {
    f32 top_left_px, top_right_px, bottom_left_px, bottom_right_px;
} RenderCmdDrawRectCornerRadii;

typedef struct RENDER_CMD_DRAW_RECT_BORDER_WIDTHS_T {
    f32 left_px, right_px, top_px, bottom_px;
} RenderCmdDrawRectBorderWidths;

typedef enum RENDER_CMD_DRAW_RECT_MODE_E {
#define X_RENDER_CMD_DRAW_RECT_MODES \
    X(FILL) \
    X(BORDER) \
    X(BLIT) \
    X(COUNT)
#define X(mode) \
    RENDER_CMD_DRAW_RECT_MODE_##mode,
    X_RENDER_CMD_DRAW_RECT_MODES
#undef X
} RenderCmdDrawRectMode;

typedef struct RENDER_CMD_DRAW_RECT_T {
    Framebuffer *dst;
    f32x4 dst_region;
    RenderCmdDrawRectCornerRadii corner_radii;
    RenderCmdDrawRectMode mode;

    union {
        struct {
            rgba8 color;
        } fill;

        struct {
            rgba8 color;
            RenderCmdDrawRectBorderWidths widths;
        } border;

        struct {
            Framebuffer *src;
            rgba8 tint;
        } blit;
    } data;
} RenderCmdDrawRect;

typedef struct RENDER_CMD_DRAW_TEXT_T {
    Framebuffer *dst_fb;
    string_view text;
    f32x4 dst_fb_region;
    FontSets *font_sets;
    FontStyle font_style;
    rgba8 text_color;
    i32 font_height_px;
    i32 letter_spacing_px;
    i32 line_height_px;
} RenderCmdDrawText;

typedef enum RENDER_CMD_DRAW_MODEL_CULL_MODE_E {
#define X_RENDER_CMD_DRAW_MODEL_CULL_MODES \
    X(BACK) \
    X(FRONT) \
    X(NONE) \
    X(COUNT)
#define X(mode) \
    RENDER_CMD_DRAW_MODEL_CULL_MODE_##mode,
    X_RENDER_CMD_DRAW_MODEL_CULL_MODES
#undef X
} RenderCmdDrawModelCullMode;

typedef struct RENDER_CMD_DRAW_MODEL_T {
    Framebuffer *dst_framebuffer;

    Model *model;

    VertexShaderFnPtr vert_shader_fn;
    FragmentShaderFnPtr frag_shader_fn;

    struct {
        u8 *bytes;
        i64 len;
    } shader_data;

    ShaderDataInitFnPtr shader_data_init_fn;

    RenderCmdDrawModelCullMode cull_mode;
} RenderCmdDrawModel;

typedef struct RENDER_CMD_SCISSOR_T {
    Framebuffer *framebuffer;
    f32x4 region;
} RenderCmdScissor;

typedef struct RENDER_CMD_PRESENT_T {
    WindowHandle *window;
    Framebuffer *framebuffer;
} RenderCmdPresent;

typedef struct RENDER_CMD_T {
    RenderCmdType type;

    union {
        RenderCmdDrawRect draw_rect;
        RenderCmdDrawText draw_text;
        RenderCmdScissor scissor;
        RenderCmdDrawModel draw_model;
        RenderCmdPresent present;
    } data;
} RenderCmd;

typedef struct RENDER_CMDS_T {
    ARRAY_FIELDS(RenderCmd)
} RenderCmds;

Renderer *renderer_create(const RendererCreateInfo *create_info);

void renderer_execute(Renderer *renderer, RenderCmds *cmds);

void renderer_destroy(Renderer **renderer);

void renderer_idle(Renderer *renderer);

#endif //ALTCORE_RENDERER_H
