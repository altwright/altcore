//
// Created by wright on 5/10/26.
//

#include "pixels.h"
#include "pixels.impl.h"
#include "../debug.h"
#include "../maths.h"

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

static void alpha_blend_8888(u8 dst[4], const u8 src[4], i64 alpha_idx) {
    if (src[alpha_idx] == 0) {
        return;
    }

    if (src[alpha_idx] == 0xff) {
        memcpy(dst, src, 4);
    } else {
        f32x4 f_src = {}, f_dst = {};

        for (i64 cmp_idx = 0; cmp_idx < 4; cmp_idx++) {
            f_src.data[cmp_idx] = (f32) src[cmp_idx] / 255.f;
            f_dst.data[cmp_idx] = (f32) dst[cmp_idx] / 255.f;
        }

        f32x4 alpha_src = f32x4_scale(f_src, f_src.data[alpha_idx]);
        f32x4 inv_alpha_dst = f32x4_scale(f_dst, 1.f - f_src.data[alpha_idx]);
        f_dst = f32x4_add(alpha_src, inv_alpha_dst);

        for (i64 cmp_idx = 0; cmp_idx < 4; cmp_idx++) {
            dst[cmp_idx] = (u8) (f_dst.data[cmp_idx] * 255.f);
        }
    }
}

void pixel_set(Pixel *dst_px, const Pixel *src_px) {
    if (dst_px->format == src_px->format) {
        switch (dst_px->format) {
            case PIXEL_FORMAT_RGBA8: {
                auto dst = (rgba8 *) dst_px->start;
                auto src = (const rgba8 *) src_px->start;

                alpha_blend_8888(dst->data, src->data, offsetof(rgba8, a));
                break;
            }
            case PIXEL_FORMAT_ARGB8: {
                auto dst = (argb8 *) dst_px->start;
                auto src = (const argb8 *) src_px->start;

                alpha_blend_8888(dst->data, src->data, offsetof(argb8, a));
                break;
            }
            default:
                crash_msg("Unhandled pixel format %d\n", dst_px->format);
                break;
        }
    } else {
        switch (dst_px->format) {
            case PIXEL_FORMAT_RGBA8: {
                auto dst = (rgba8 *) dst_px->start;
                switch (src_px->format) {
                    case PIXEL_FORMAT_ARGB8: {
                        auto src = (const argb8 *) src_px->start;

                        rgba8 new_src = {
                            .r = src->r,
                            .g = src->g,
                            .b = src->b,
                            .a = src->a
                        };

                        alpha_blend_8888(dst->data, new_src.data, offsetof(rgba8, a));

                        break;
                    }
                    case PIXEL_FORMAT_ABGR8: {
                        auto src = (const abgr8 *) src_px->start;
                        if (src->a == 0) {
                            break;
                        }

                        rgba8 new_src = {
                            .r = src->r,
                            .g = src->g,
                            .b = src->b,
                            .a = src->a
                        };

                        alpha_blend_8888(dst->data, new_src.data, offsetof(rgba8, a));

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
