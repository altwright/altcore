//
// Created by wright on 5/8/26.
//

#include "clear.h"

#include "../../memory.h"
#include "../../debug.h"

typedef union CLEAR_COLOR_U {
    RGBA8888 rgba;
    ARGB8888 argb;
} ClearColor;

void cmd_soft_single_clear(u8* pixel_bytes, PixelFormat pixel_format, i32x2 size, i64 pitch_bytes, RGBA8888 rgba) {
    i32 pixel_size = (i32)pixels_get_size(pixel_format);

    ClearColor clear_color = {};
    switch (pixel_format) {
        case PIXEL_FORMAT_RGBA_8888: {
            clear_color.rgba = rgba;
            break;
        }
        case PIXEL_FORMAT_ARGB_8888: {
            clear_color.argb.a = rgba.a;
            clear_color.argb.r = rgba.r;
            clear_color.argb.g = rgba.g;
            clear_color.argb.b = rgba.b;
            break;
        }
        default:
            crash_msg("Unhandled pixel format %d\n", pixel_format);
            break;
    }

    for (i32 y_idx = 0; y_idx < size.y; y_idx++) {
        for (i32 x_idx = 0; x_idx < size.x; x_idx++) {
            u8* pixel_start = pixel_bytes + (y_idx * pitch_bytes + x_idx * pixel_size);

            switch (pixel_format) {
                case PIXEL_FORMAT_RGBA_8888: {
                    RGBA8888* pixel = (RGBA8888*)pixel_start;
                    *pixel = clear_color.rgba;
                    break;
                }
                case PIXEL_FORMAT_ARGB_8888: {
                    ARGB8888* pixel = (ARGB8888*)pixel_start;
                    *pixel = clear_color.argb;
                    break;
                }
                default:
                    crash_msg("Unhandled pixel format %d\n", pixel_format);
                    break;
            }
        }
    }
}
