//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_FRAMEBUFFER_H
#define ALTCORE_FRAMEBUFFER_H

#include "../types.h"
#include "pixels.h"

struct FRAMEBUFFER_T;
typedef struct FRAMEBUFFER_T Framebuffer;

struct SWAPCHAIN_BUFFER_T;

typedef struct FRAMEBUFFER_DATA_T {
    PixelFormat format;
    u8 *pixels;
    iVec2 size;
    i32 pitch;
} FramebufferData;

FramebufferData framebuffer_get_data(Framebuffer* framebuffer);

void framebuffer_data_set_pixel(FramebufferData fb_data, i32 x, i32 y, rgba8888 rgba);

#endif //ALTCORE_FRAMEBUFFER_H
