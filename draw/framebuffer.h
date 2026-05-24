//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_FRAMEBUFFER_H
#define ALTCORE_FRAMEBUFFER_H

#include "../types.h"
#include "pixels.h"

struct FRAMEBUFFER_T;
typedef struct FRAMEBUFFER_T Framebuffer;

typedef enum FRAMEBUFFER_TYPE_E {
#ifndef X_FRAMEBUFFER_TYPES
#define X_FRAMEBUFFER_TYPES \
    X(PIXEL) \
    X(VERTEX) \
    X(INDEX) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    FRAMEBUFFER_TYPE_##type,
#endif
    X_FRAMEBUFFER_TYPES
#undef X
} FramebufferType;

typedef struct PIXEL_BUFFER_CREATE_INFO_T {
    PixelFormat format;
    i32x2 size;
} PixelBufferCreateInfo;

typedef struct FRAMEBUFFER_CREATE_INFO_T {
    FramebufferType type;

    union {
        PixelBufferCreateInfo pixel_buf;
    } data;
} FramebufferCreateInfo;

typedef struct PIXEL_BUFFER_INFO_T {
    PixelFormat format;
    i32x2 size;
    i64 pitch_bytes;
} PixelBufferInfo;

typedef struct FRAMEBUFFER_INFO_T {
    FramebufferType type;

    union {
        PixelBufferInfo pixel_buf;
    } data;
} FramebufferInfo;

Framebuffer *framebuffer_create(const FramebufferCreateInfo *create_info);

void framebuffer_destroy(Framebuffer *fb);

FramebufferInfo framebuffer_get_info(Framebuffer *fb);

#endif //ALTCORE_FRAMEBUFFER_H
