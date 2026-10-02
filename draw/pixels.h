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

#ifndef RGBA8_NIL
#define RGBA8_NIL (rgba8){.r = 0, .g = 0, .b = 0, .a = 0}
#endif

#ifndef RGBA8_RED
#define RGBA8_RED (rgba8){.r = 0xff, .g = 0, .b = 0, .a = 0xff}
#endif

#ifndef RGBA8_GREEN
#define RGBA8_GREEN (rgba8){.r = 0xff, .g = 0xff, .b = 0, .a = 0xff}
#endif

#ifndef RGBA8_BLUE
#define RGBA8_BLUE (rgba8){.r = 0, .g = 0, .b = 0xff, .a = 0xff}
#endif

#ifndef RGBA8_BLACK
#define RGBA8_BLACK (rgba8){.r = 0, .g = 0, .b = 0, .a = 0xff}
#endif

#ifndef RGBA8_WHITE
#define RGBA8_WHITE (rgba8){.r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff}
#endif

#ifndef RGBA8_YELLOW
#define RGBA8_YELLOW (rgba8){.r = 0xff, .g = 0xff, .b = 0, .a = 0xff}
#endif

#ifndef RGBA8_MAGENTA
#define RGBA8_MAGENTA (rgba8){.r = 0xff, .g = 0, .b = 0xff, .a = 0xff}
#endif

#ifndef RGBA8_CYAN
#define RGBA8_CYAN (rgba8){.r = 0, .g = 0xff, .b = 0xff, .a = 0xff}
#endif

i64 pixel_size(PixelFormat format);

void pixel_set(Pixel *dst_px, const Pixel *src_px);

#endif //ALTCORE_PIXELS_H
