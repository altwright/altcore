//
// Created by wright on 5/6/26.
//

#ifndef ALTCORE_PIXELS_H
#define ALTCORE_PIXELS_H

#include "../types.h"

typedef enum PIXEL_FORMAT_E {
#ifndef X_PIXEL_FORMATS
#define X_PIXEL_FORMATS \
    X(ARGB_8888) \
    X(RGBA_8888) \
    X(COUNT)
#endif
#ifndef X
#define X(fmt) \
    PIXEL_FORMAT_##fmt,
#endif
    X_PIXEL_FORMATS
#undef X
} PixelFormat;

typedef struct RGBA_8888_T {
    u8 a, b, g, r;
} RGBA8888;

typedef struct ARGB_8888_T {
    u8 b, g, r, a;
} ARGB8888;

typedef union PIXEL_U {
    RGBA8888 rgba;
    ARGB8888 argb;
} PixelColor;

PixelColor pixels_convert_rgba(PixelFormat format, RGBA8888 rgba);

i32 pixels_get_size(PixelFormat pixel_format);

void pixels_set(u8* px_start, PixelFormat format, PixelColor px);

#endif //ALTCORE_PIXELS_H
