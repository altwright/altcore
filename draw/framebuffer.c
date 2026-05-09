//
// Created by wright on 5/3/26.
//

#include "framebuffer.h"

#include "../memory.h"
#include "../debug.h"
#include "window.h"

typedef enum FRAMEBUFFER_TYPE_E {
#ifndef X_FRAMEBUFFER_TYPES
#define X_FRAMEBUFFER_TYPES \
    X(PIXEL_BUFFER) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    FRAMEBUFFER_TYPE_##type,
#endif
    X_FRAMEBUFFER_TYPES
#undef X
} FramebufferType;

struct FRAMEBUFFER_T {
    FramebufferType type;
};

FramebufferData framebuffer_get_data(Framebuffer* framebuffer) {
    FramebufferData data = {};

    return data;
}

void framebuffer_data_set_pixel(FramebufferData fb_data, i32 x, i32 y, rgba8888 rgba) {
    switch (fb_data.format) {
        case PIXEL_FORMAT_ARGB_8888: {
            argb8888 *pixel = ((argb8888 *) fb_data.pixels) + (y * fb_data.pitch + x);
            pixel->a = rgba.a;
            pixel->r = rgba.r;
            pixel->g = rgba.g;
            pixel->b = rgba.b;
            break;
        }
        case PIXEL_FORMAT_RGBA_8888: {
            rgba8888 *pixel = ((rgba8888 *) fb_data.pixels) + (y * fb_data.pitch + x);
            *pixel = rgba;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer format\n");
            break;
    }
}
