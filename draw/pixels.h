//
// Created by wright on 5/6/26.
//

#ifndef ALTCORE_PIXELS_H
#define ALTCORE_PIXELS_H

typedef enum PIXEL_FORMAT_E {
#ifndef X_PIXEL_FORMATS
#define X_PIXEL_FORMATS \
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

#endif //ALTCORE_PIXELS_H
