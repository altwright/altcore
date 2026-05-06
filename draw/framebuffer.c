//
// Created by wright on 5/3/26.
//

#include "draw/framebuffer.h"

#include "memory.h"
#include "draw/window.h"

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
