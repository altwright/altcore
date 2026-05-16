//
// Created by wright on 5/10/26.
//

#include "pixels.h"
#include "pixels_impl.h"
#include "../debug.h"

i64 pixels_size(PixelFormat pixel_format) {
    i64 pixel_size = 0;

    switch (pixel_format) {
        case PIXEL_FORMAT_ARGB_8888: {
            pixel_size = sizeof(ARGB8888);
            break;
        }
        case PIXEL_FORMAT_RGBA_8888: {
            pixel_size = sizeof(RGBA8888);
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", pixel_format);
            break;
    }

    return pixel_size;
}

SDL_PixelFormat pixels_impl_get_format(PixelFormat format) {
    SDL_PixelFormat sdl_format = {};

    switch (format) {
        case PIXEL_FORMAT_ARGB_8888: {
            sdl_format = SDL_PIXELFORMAT_ARGB8888;
            break;
        }
        case PIXEL_FORMAT_RGBA_8888: {
            sdl_format = SDL_PIXELFORMAT_RGBA8888;
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", format);
            break;
    }

    return sdl_format;
}
