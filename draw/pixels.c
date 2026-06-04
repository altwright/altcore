//
// Created by wright on 5/10/26.
//

#include "pixels.h"
#include "pixels_impl.h"
#include "../debug.h"

i32 pixels_get_size(PixelFormat pixel_format) {
    i32 pixel_size = 0;

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

SDL_PixelFormat pixels_impl_to_sdl_format(PixelFormat format) {
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

PixelFormat pixels_impl_from_sdl_format(SDL_PixelFormat sdl_format) {
    PixelFormat pixel_format = PIXEL_FORMAT_COUNT;

    switch (sdl_format) {
        case SDL_PIXELFORMAT_RGBA8888: {
            pixel_format = PIXEL_FORMAT_RGBA_8888;
            break;
        }
        case SDL_PIXELFORMAT_ARGB8888:
        case SDL_PIXELFORMAT_XRGB8888: {
            pixel_format = PIXEL_FORMAT_ARGB_8888;
            break;
        }
        default:
            crash_msg("Unhandled SDL pixel format %d\n", sdl_format);
            break;
    }

    return pixel_format;
}

PixelColor pixels_convert_rgba(PixelFormat format, RGBA8888 rgba) {
    PixelColor px = {};

    switch (format) {
        case PIXEL_FORMAT_RGBA_8888: {
            px.rgba = rgba;
            break;
        }
        case PIXEL_FORMAT_ARGB_8888: {
            px.argb.r = rgba.r;
            px.argb.g = rgba.g;
            px.argb.b = rgba.b;
            px.argb.a = rgba.a;
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", format);
            break;
    }

    return px;
}

void pixels_set(u8* px_start, PixelFormat format, PixelColor px) {
    switch (format) {
        case PIXEL_FORMAT_RGBA_8888: {
            RGBA8888* pixel = (RGBA8888*)px_start;
            *pixel = px.rgba;
            break;
        }
        case PIXEL_FORMAT_ARGB_8888: {
            ARGB8888* pixel = (ARGB8888*)px_start;
            *pixel = px.argb;
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", format);
            break;
    }
}
