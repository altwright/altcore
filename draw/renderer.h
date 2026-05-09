//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_RENDERER_H
#define ALTCORE_RENDERER_H

#include "../types.h"
#include "framebuffer.h"
#include "window.h"
#include "../barrier.h"

struct RENDERER_T;
typedef struct RENDERER_T Renderer;

typedef struct RENDERER_CREATE_INFO_T {
} RendererCreateInfo;

typedef enum RENDER_CMD_TYPE_E {
#ifndef X_RENDER_CMD_TYPES
#define X_RENDER_CMD_TYPES \
    X(CLEAR) \
    X(SWAPCHAIN_BUF_PRESENT) \
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
    RenderBufferType buf_type;

    union {
        Framebuffer *framebuffer;
        SwapchainBuffer *swapchain_buf;
    } buf;

    rgba8888 rgba;
} RenderCmdClear;

typedef struct RENDER_CMD_SWAPCHAIN_BUF_PRESENT_T {
    SwapchainBuffer *swapchain_buf;
    Barrier *barrier;
} RenderCmdSwapchainBufPresent;

typedef struct RENDER_CMD_T {
    RenderCmdType type;

    union {
        RenderCmdClear clear;
        RenderCmdSwapchainBufPresent swapchain_buf_present;
    } data;
} RenderCmd;

typedef struct RENDER_CMD_BUFFER_T {
    ARRAY_FIELDS(RenderCmd)
} RenderCmdBuffer;

Renderer *renderer_create(const RendererCreateInfo *create_info);

void renderer_execute_cmd_buf(Renderer *renderer, RenderCmdBuffer cmd_buf);

void renderer_destroy(Renderer *renderer);

#endif //ALTCORE_RENDERER_H
