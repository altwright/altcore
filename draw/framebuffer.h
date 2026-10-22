//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_FRAMEBUFFER_H
#define ALTCORE_FRAMEBUFFER_H

#include "../types.h"
#include "pixels.h"

typedef struct FRAMEBUFFER_T Framebuffer;
typedef struct RENDERER_T Renderer;

typedef struct FRAMEBUFFERS_T {
    ARRAY_FIELDS(Framebuffer*)
} Framebuffers;

typedef enum FRAMEBUFFER_TYPE_E {
#ifndef X_FRAMEBUFFER_TYPES
#define X_FRAMEBUFFER_TYPES \
    X(PIXEL) \
    X(DEPTH) \
    X(STENCIL) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    FRAMEBUFFER_TYPE_##type,
#endif
    X_FRAMEBUFFER_TYPES
#undef X
} FramebufferType;

typedef enum FRAMEBUFFER_CREATE_ACTION_TYPE_E {
#define X_FRAMEBUFFER_CREATE_ACTION_TYPES \
    X(NEW) \
    X(LOAD) \
    X(COUNT)
#define X(action) \
    FRAMEBUFFER_CREATE_ACTION_TYPE_##action,
    X_FRAMEBUFFER_CREATE_ACTION_TYPES
#undef X
} FramebufferCreateActionType;

typedef enum FRAMEBUFFER_LOAD_COMPRESSION_TYPE_E {
#define X_FRAMEBUFFER_LOAD_COMPRESSION_TYPES \
    X(NONE) \
    X(JPEG) \
    X(PNG) \
    X(COUNT)
#define X(type) \
    FRAMEBUFFER_LOAD_COMPRESSION_TYPE_##type,
    X_FRAMEBUFFER_LOAD_COMPRESSION_TYPES
#undef X
} FramebufferLoadCompressionType;

typedef struct FRAMEBUFFER_PIXEL_CREATE_INFO_T {
    FramebufferCreateActionType type;

    union {
        struct {
            PixelFormat dst_format;
            i32x2 size;
        } new;

        struct {
            PixelFormat dst_format;
            FramebufferLoadCompressionType type;

            union {
                struct {
                    u8 *data;
                    PixelFormat src_format;
                    i64 pitch;
                    i32x2 size;
                } none;

                struct {
                    u8 *data;
                    i64 len;
                } png;

                struct {
                    u8 *data;
                    i64 len;
                } jpeg;
            } data;
        } load;
    } data;
} FramebufferPixelCreateInfo;

typedef struct FRAMEBUFFER_CREATE_INFO_T {
    Renderer *renderer;
    FramebufferType type;

    union {
        FramebufferPixelCreateInfo pixel;
    } data;
} FramebufferCreateInfo;

typedef struct FRAMEBUFFER_PIXEL_INFO_T {
    PixelFormat format;
    i32x2 size;
    f32x4 scissor;
} FramebufferPixelInfo;

typedef struct FRAMEBUFFER_INFO_T {
    FramebufferType type;

    union {
        FramebufferPixelInfo pixel;
    } data;
} FramebufferInfo;

typedef struct FRAMEBUFFER_VIEW_T {
    f32x2 offset;
    f32x2 scale;
    f32 rotation;
    i64 texture_idx;
} FramebufferView;

typedef enum FRAMEBUFFER_SAMPLE_FILTER_MODE_E {
#define X_FRAMEBUFFER_SAMPLE_FILTER_MODES \
    X(NEAREST) \
    X(COUNT)
#define X(mode) \
    FRAMEBUFFER_SAMPLE_FILTER_MODE_##mode,
    X_FRAMEBUFFER_SAMPLE_FILTER_MODES
#undef X
} FramebufferSampleFilterMode;

typedef enum FRAMEBUFFER_SAMPLE_WRAP_MODE_E {
#define X_FRAMEBUFFER_SAMPLE_WRAP_MODES \
    X(CLAMP) \
    X(REPEAT) \
    X(MIRROR) \
    X(COUNT)
#define X(mode) \
    FRAMEBUFFER_SAMPLE_WRAP_MODE_##mode,
    X_FRAMEBUFFER_SAMPLE_WRAP_MODES
#undef X
} FramebufferSampleWrapMode;

typedef struct FRAMEBUFFER_SAMPLE_INFO_T {
    struct {
        FramebufferSampleFilterMode min;
        FramebufferSampleFilterMode mag;
    } filter;

    struct {
        FramebufferSampleWrapMode horizontal;
        FramebufferSampleWrapMode vertical;
    } wrap;
} FramebufferSampleInfo;

Framebuffer *framebuffer_create(const FramebufferCreateInfo *create_info);

void framebuffer_destroy(Framebuffer *fb);

FramebufferInfo framebuffer_get_info(const Framebuffer *fb);

rgba32f framebuffer_get_sample(Framebuffer *fb, f32x2 normalized_coord, FramebufferSampleInfo info);

#endif //ALTCORE_FRAMEBUFFER_H
