//
// Created by wright on 5/10/26.
//

#include "pixels.h"
#include "pixels.impl.h"
#include "../debug.h"

i64 pixel_size(PixelFormat format) {
    i32 size = 0;

    switch (format) {
        case PIXEL_FORMAT_ARGB8: {
            size = sizeof(argb8);
            break;
        }
        case PIXEL_FORMAT_RGBA8: {
            size = sizeof(rgba8);
            break;
        }
        case PIXEL_FORMAT_ABGR8: {
            size = sizeof(abgr8);
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", format);
            break;
    }

    return size;
}

SDL_PixelFormat pixel_impl_to_sdl_format(PixelFormat format) {
    SDL_PixelFormat sdl_format = {};

    switch (format) {
        case PIXEL_FORMAT_ARGB8: {
            sdl_format = SDL_PIXELFORMAT_ARGB8888;
            break;
        }
        case PIXEL_FORMAT_RGBA8: {
            sdl_format = SDL_PIXELFORMAT_RGBA8888;
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", format);
            break;
    }

    return sdl_format;
}

PixelFormat pixel_impl_from_sdl_format(SDL_PixelFormat sdl_format) {
    PixelFormat pixel_format = PIXEL_FORMAT_COUNT;

    switch (sdl_format) {
        case SDL_PIXELFORMAT_RGBA8888: {
            pixel_format = PIXEL_FORMAT_RGBA8;
            break;
        }
        case SDL_PIXELFORMAT_ARGB8888:
        case SDL_PIXELFORMAT_XRGB8888: {
            pixel_format = PIXEL_FORMAT_ARGB8;
            break;
        }
        default:
            crash_msg("Unhandled SDL pixel format %d\n", sdl_format);
            break;
    }

    return pixel_format;
}

void pixel_set(Pixel *dst_px, const Pixel *src_px) {
    if (dst_px->format == src_px->format) {
        switch (dst_px->format) {
            case PIXEL_FORMAT_RGBA8: {
                auto dst = (rgba8 *) dst_px->px_start;
                auto src = (const rgba8 *) src_px->px_start;

                if (src->a == 0) {
                    break;
                }

                *dst = *src;

                break;
            }
            case PIXEL_FORMAT_ARGB8: {
                auto dst = (argb8 *) dst_px->px_start;
                auto src = (const argb8 *) src_px->px_start;

                if (src->a == 0) {
                    break;
                }

                *dst = *src;
                break;
            }
            default:
                crash_msg("Unhandled pixel format %d\n", dst_px->format);
                break;
        }
    } else {
        switch (dst_px->format) {
            case PIXEL_FORMAT_RGBA8: {
                auto dst = (rgba8 *) dst_px->px_start;
                switch (src_px->format) {
                    case PIXEL_FORMAT_ARGB8: {
                        auto src = (const argb8 *) src_px->px_start;
                        if (src->a == 0) {
                            break;
                        }

                        dst->a = src->a;
                        dst->r = src->r;
                        dst->g = src->g;
                        dst->b = src->b;

                        break;
                    }
                    case PIXEL_FORMAT_ABGR8: {
                        auto src = (const abgr8 *) src_px->px_start;
                        if (src->a == 0) {
                            break;
                        }

                        dst->a = src->a;
                        dst->r = src->r;
                        dst->g = src->g;
                        dst->b = src->b;

                        break;
                    }
                    default:
                        crash_msg("Unhandled src pixel format %d\n", src_px->format);
                        break;
                }
                break;
            }
            default:
                crash_msg("Unhandled dst pixel format %d\n", dst_px->format);
                break;
        }
    }
}
