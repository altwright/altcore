//
// Created by wright on 5/6/26.
//

#ifndef ALTCORE_PIXELS_H
#define ALTCORE_PIXELS_H

#include "../types.h"

typedef enum PIXEL_FORMAT_E {
#ifndef X_PIXEL_FORMATS
#define X_PIXEL_FORMATS \
    X(RGBA8) \
    X(ABGR8) \
    X(ARGB8) \
    X(COUNT)
#endif
#ifndef X
#define X(fmt) \
    PIXEL_FORMAT_##fmt,
#endif
    X_PIXEL_FORMATS
#undef X
} PixelFormat;

typedef struct RGBA8_T {
    union {
        struct {
            u8 a, b, g, r;
        };

        u8 data[4];
    };
} rgba8;

typedef struct ABGR8_T {
    u8 r, g, b, a;
} abgr8;

typedef struct ARGB8_T {
    union {
        struct {
            u8 b, g, r, a;
        };

        u8 data[4];
    };
} argb8;

typedef struct PIXEL_T {
    PixelFormat format;
    u8 *start;
} Pixel;

i64 pixel_size(PixelFormat format);

void pixel_set(Pixel *dst_px, const Pixel *src_px);

#endif //ALTCORE_PIXELS_H
