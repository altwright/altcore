//
// Created by wright on 9/25/26.
//

#ifndef BRITANNICUS_IMAGE_H
#define BRITANNICUS_IMAGE_H

#include "framebuffer.h"

typedef enum IMAGE_COMPRESSION_TYPE_E {
#define X_IMAGE_COMPRESSION_TYPES \
    X(PNG) \
    X(COUNT)
#define X(type) \
    IMAGE_COMPRESSION_TYPE_##type,
    X_IMAGE_COMPRESSION_TYPES
#undef X
} ImageCompressionType;

typedef struct IMAGE_CREATE_INFO_T {
    ImageCompressionType compression;

    struct {
        u8 *data;
        i64 len;
    } bytes;

    PixelFormat dst_format;
} ImageLoadInfo;

Framebuffer *image_load(const ImageLoadInfo *info);

#endif //BRITANNICUS_IMAGE_H
