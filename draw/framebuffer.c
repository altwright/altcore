//
// Created by wright on 5/3/26.
//

#include "framebuffer.h"

#include <assert.h>
#include <threads.h>

#include "../memory.h"
#include "../debug.h"
#include "window.h"
#include "framebuffer_impl.h"

typedef struct PIXEL_BUFFER_T {
    PixelFormat format;
    u8 *bytes;
    iVec2 size;
} PixelBuffer;

struct FRAMEBUFFER_T {
    FramebufferType type;
    union {
        PixelBuffer pixel_buf;
    } data;

    mtx_t lock;
};

Framebuffer *framebuffer_create(const FramebufferCreateInfo *create_info) {
    Framebuffer *fb = alt_malloc(sizeof(*fb));

    *fb = (Framebuffer){};

    switch (create_info->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            fb->data.pixel_buf.format = create_info->data.pixel_buf.format;
            fb->data.pixel_buf.size = create_info->data.pixel_buf.size;
            fb->data.pixel_buf.bytes = alt_calloc(
                fb->data.pixel_buf.size.x * fb->data.pixel_buf.size.y,
                pixels_get_size(create_info->data.pixel_buf.format)
            );
            assert(fb->data.pixel_buf.bytes);
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", create_info->type);
            break;
    }

    mtx_init(&fb->lock, mtx_plain);

    return fb;
}

void framebuffer_destroy(Framebuffer *fb) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            alt_free(fb->data.pixel_buf.bytes);
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }

    mtx_destroy(&fb->lock);

    alt_free(fb);
}

FramebufferInfo framebuffer_get_info(Framebuffer *fb) {
    FramebufferInfo info = {};

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            info.type = FRAMEBUFFER_TYPE_PIXEL;
            info.data.pixel_buf.format = fb->data.pixel_buf.format;
            info.data.pixel_buf.size = fb->data.pixel_buf.size;
            info.data.pixel_buf.pitch_bytes = fb->data.pixel_buf.size.x * pixels_get_size(fb->data.pixel_buf.format);
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }

    return info;
}

void framebuffer_impl_lock(Framebuffer *fb) {
    mtx_lock(&fb->lock);
}

void framebuffer_impl_unlock(Framebuffer *fb) {
    mtx_unlock(&fb->lock);
}

u8* framebuffer_impl_get_bytes(Framebuffer *fb) {
    u8* bytes = nullptr;

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            bytes = fb->data.pixel_buf.bytes;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }

    return bytes;
}
