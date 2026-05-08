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
    X(SWAPCHAIN_BUFFER) \
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

    union {
        struct {
            SwapchainBuffer* buf;
            SwapchainBufferData data;
        } swapchain;
    } data;
};

Framebuffer* framebuffer_open_swapchain_buf(SwapchainBuffer* swapchain_buf) {
    Framebuffer* framebuffer = alt_malloc(sizeof(Framebuffer));

    *framebuffer = (Framebuffer) {
        .type = FRAMEBUFFER_TYPE_SWAPCHAIN_BUFFER,
        .data = {
            .swapchain = {
                .buf = swapchain_buf,
                .data = swapchain_open(swapchain_buf),
            },
        },
    };

    return framebuffer;
}

void framebuffer_close(Framebuffer* framebuffer) {
    swapchain_close(framebuffer->data.swapchain.buf, &framebuffer->data.swapchain.data);

    alt_free(framebuffer);
}

FramebufferData frambuffer_get_data(Framebuffer* framebuffer) {
    FramebufferData data = {};

    switch (framebuffer->type) {
        case FRAMEBUFFER_TYPE_SWAPCHAIN_BUFFER: {
            SwapchainBufferData* swapchain_data = &framebuffer->data.swapchain.data;
            data.format = swapchain_data->format;
            data.pitch = swapchain_data->pitch;
            data.size = (iVec2){swapchain_data->width, swapchain_data->height};
            data.pixels = swapchain_data->pixels;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", framebuffer->type);
            break;
    }

    return data;
}

void framebuffer_data_set_pixel(FramebufferData *fb_data, i32 x, i32 y, uVec4 rgba) {
    switch (fb_data->format) {
        case PIXEL_FORMAT_RGBA_8888: {
            uVec4 *pixel = ((uVec4 *) fb_data->pixels) + (y * fb_data->pitch + x);
            *pixel = rgba;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer format\n");
            break;
    }
}
