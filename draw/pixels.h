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
    u8 r, g, b, a;
} RGBA8888;

typedef struct ARGB_8888_T {
    u8 a, r, g, b;
} ARGB8888;

i64 pixels_get_size(PixelFormat pixel_format);

#endif //ALTCORE_PIXELS_H
