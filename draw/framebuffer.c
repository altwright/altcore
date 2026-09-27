//
// Created by wright on 5/3/26.
//

#include "framebuffer.h"

#include <assert.h>
#include <threads.h>

#include "../memory.h"
#include "../debug.h"
#include "window.h"
#include "framebuffer.impl.h"

typedef struct PIXEL_BUFFER_T {
    PixelFormat format;
    u8 *bytes;
    i32x2 size;
    i32x4 scissor;
} PixelBuffer;

struct FRAMEBUFFER_T {
    FramebufferType type;

    union {
        PixelBuffer pixel_buf;
    } data;
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
                pixel_size(create_info->data.pixel_buf.format)
            );
            fb->data.pixel_buf.scissor = (i32x4){
                .start_x = 0,
                .start_y = 0,
                .width = create_info->data.pixel_buf.size.width,
                .height = create_info->data.pixel_buf.size.height
            };
            assert(fb->data.pixel_buf.bytes);
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", create_info->type);
            break;
    }

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

    alt_free(fb);
}

FramebufferInfo framebuffer_get_info(const Framebuffer *fb) {
    FramebufferInfo info = {};

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            info.type = FRAMEBUFFER_TYPE_PIXEL;
            info.data.pixel_buf.format = fb->data.pixel_buf.format;
            info.data.pixel_buf.size = fb->data.pixel_buf.size;
            info.data.pixel_buf.pitch_bytes = fb->data.pixel_buf.size.x * pixel_size(fb->data.pixel_buf.format);
            info.data.pixel_buf.scissor = fb->data.pixel_buf.scissor;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }

    return info;
}

u8 *framebuffer_impl_get_bytes(Framebuffer *fb) {
    u8 *bytes = nullptr;

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

void framebuffer_set_scissor(Framebuffer *fb, f32x4 region) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            fb->data.pixel_buf.scissor = ftoi32x4(region);
            i32x4 *scissor = &fb->data.pixel_buf.scissor;
            if (scissor->start_x < 0) {
                scissor->width += scissor->start_x;
                scissor->start_x = 0;
            }
            if (scissor->start_y < 0) {
                scissor->height += scissor->start_y;
                scissor->start_y = 0;
            }
            if (scissor->width < 0) {
                scissor->width = 0;
            }
            if (scissor->height < 0) {
                scissor->height = 0;
            }
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }
}
