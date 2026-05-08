//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_RENDERER_H
#define ALTCORE_RENDERER_H

#include "../types.h"
#include "framebuffer.h"
#include "../arenas.h"

struct RENDERER_T;
typedef struct RENDERER_T Renderer;

typedef struct RENDERER_CREATE_INFO_T {
} RendererCreateInfo;

typedef enum RENDER_CMD_TYPE_E {
#ifndef X_RENDER_CMD_TYPES
#define X_RENDER_CMD_TYPES \
    X(FRAMEBUFFER_TRANSITION) \
    X(CLEAR) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    RENDER_CMD_TYPE_##type,
#endif
    X_RENDER_CMD_TYPES
#undef X
} RenderCmdType;

typedef struct RENDER_CMD_CLEAR_T {
    Framebuffer *framebuffer;
    uVec4 rgba;
} RenderCmdClear;

typedef enum FRAMEBUFFER_TRANSITION_TYPE_E {
#ifndef X_FRAMEBUFFER_TRANSITION_TYPES
#define X_FRAMEBUFFER_TRANSITION_TYPES \
    X(BLIT) \
    X(PRESENT) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    FRAMEBUFFER_TRANSITION_TYPE_##type,
#endif
    X_FRAMEBUFFER_TRANSITION_TYPES
#undef X
} FramebufferTransitionType;

typedef struct RENDER_CMD_FRAMEBUFFER_TRANSITION_T {
    Framebuffer *framebuffer;
    FramebufferTransitionType to;
} RenderCmdFramebufferTransition;

typedef struct RENDER_CMD_T {
    RenderCmdType type;

    union {
        RenderCmdClear clear;
        RenderCmdFramebufferTransition fb_transition;
    } data;
} RenderCmd;

typedef struct RENDER_CMD_BUFFER_T {
    ARRAY_FIELDS(RenderCmd)
} RenderCmdBuffer;

Renderer *renderer_create(const RendererCreateInfo *create_info);

void renderer_execute_cmd_bufs(Renderer *renderer, RenderCmdBuffer cmd_bufs[], i32 cmd_bufs_len);

void renderer_destroy(Renderer *renderer);

#endif //ALTCORE_RENDERER_H
