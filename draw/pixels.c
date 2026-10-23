//
// Created by wright on 5/10/26.
//

#include "pixels.h"
#include "pixels.impl.h"
#include "../debug.h"
#include "../maths.h"

constexpr f32 kEpsilon32f = 0.001f;

i64 pixel_size(PixelFormat format) {
    i32 size = 0;

    switch (format) {
        case PIXEL_FORMAT_BGRA8: {
            size = sizeof(bgra8);
            break;
        }
        case PIXEL_FORMAT_RGBA8: {
            size = sizeof(rgba8);
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
        case PIXEL_FORMAT_RGBA8: {
            sdl_format = SDL_PIXELFORMAT_ABGR8888;
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
        case SDL_PIXELFORMAT_ABGR8888: {
            pixel_format = PIXEL_FORMAT_RGBA8;
            break;
        }
        case SDL_PIXELFORMAT_ARGB8888:
        case SDL_PIXELFORMAT_XRGB8888: {
            pixel_format = PIXEL_FORMAT_BGRA8;
            break;
        }
        default:
            crash_msg("Unhandled SDL pixel format %d\n", sdl_format);
            break;
    }

    return pixel_format;
}

static void alpha_blend_8(u8 dst[4], const u8 src[4], i64 alpha_idx) {
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

static void alpha_blend_32f(f32 dst[4], const f32 src[4], i64 alpha_idx) {
    f32x4 new_src = {};
    f32x4 new_dst = {};

    memcpy(new_src.data, src, sizeof(f32x4));
    memcpy(new_dst.data, dst, sizeof(f32x4));

    f32x4 alpha_src = f32x4_scale(new_src, new_src.data[alpha_idx]);
    f32x4 inv_alpha_dst = f32x4_scale(new_dst, 1.f - new_src.data[alpha_idx]);
    new_dst = f32x4_add(alpha_src, inv_alpha_dst);

    memcpy(dst, new_dst.data, sizeof(f32x4));
}

void pixel_set(Pixel *dst_px, const Pixel *src_px) {
    switch (dst_px->format) {
        case PIXEL_FORMAT_RGBA8: {
            auto dst = (rgba8 *) dst_px->start;
            rgba8 new_src = {};
            switch (src_px->format) {
                case PIXEL_FORMAT_RGBA8: {
                    new_src = *(const rgba8 *) src_px->start;
                    break;
                }
                case PIXEL_FORMAT_BGRA8: {
                    auto src = (const bgra8 *) src_px->start;

                    new_src = (rgba8){
                        .r = src->r,
                        .g = src->g,
                        .b = src->b,
                        .a = src->a
                    };

                    break;
                }
                case PIXEL_FORMAT_RGBA32F: {
                    auto src = (const rgba32f *) src_px->start;
                    if (src->a == 0) {
                        break;
                    }

                    for (i64 channel_idx = 0; channel_idx < 4; channel_idx++) {
                        new_src.data[channel_idx] = (u8) (src->data[channel_idx] * 255.f);
                    }

                    break;
                }
                default:
                    crash_msg("Unhandled src pixel format %d\n", src_px->format);
                    break;
            }

            alpha_blend_8(dst->data, new_src.data, offsetof(rgba8, a) / sizeof(u8));

            break;
        }
        case PIXEL_FORMAT_BGRA8: {
            auto dst = (bgra8 *) dst_px->start;
            bgra8 new_src = {};

            switch (src_px->format) {
                case PIXEL_FORMAT_BGRA8: {
                    new_src = *(const bgra8 *) src_px->start;
                    break;
                }
                default:
                    crash_msg("Unhandled src pixel format\n");
                    break;
            }

            alpha_blend_8(dst->data, new_src.data, offsetof(bgra8, a) / sizeof(u8));

            break;
        }
        case PIXEL_FORMAT_RGBA32F: {
            auto dst = (rgba32f *) dst_px->start;
            rgba32f new_src = {};
            switch (src_px->format) {
                case PIXEL_FORMAT_RGBA8: {
                    auto src = (rgba8 *) src_px->start;
                    for (i64 cmp_idx = 0; cmp_idx < 4; cmp_idx++) {
                        new_src.data[cmp_idx] = (f32) src->data[cmp_idx] / 255.f;
                    }
                    break;
                }
                default:
                    crash_msg("Unhandled pixel format %d\n", src_px->format);
                    break;
            }

            alpha_blend_32f(dst->data, new_src.data, offsetof(rgba32f, a) / sizeof(f32));
            break;
        }
        default:
            crash_msg("Unhandled dst pixel format %d\n", dst_px->format);
            break;
    }
}
