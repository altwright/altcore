//
// Created by wright on 10/21/26.
//

#ifndef BRITANNICUS_TEXTURE_H
#define BRITANNICUS_TEXTURE_H

#include "../types.h"
#include "pixels.h"

typedef struct TEXTURE_T Texture;

typedef struct TEXTURE_VIEW_T {
    f32x2 offset;
    f32x2 scale;
    f32 rotation;
    i64 texture_idx;
} TextureView;

typedef enum TEXTURE_COMPRESSION_TYPE_E {
#define X_TEXTURE_COMPRESSION_TYPES \
    X(PNG) \
    X(BASISU) \
    X(COUNT)
#define X(type) \
    TEXTURE_COMPRESSION_TYPE_##type,
    X_TEXTURE_COMPRESSION_TYPES
#undef X
} TextureCompressionType;

typedef struct TEXTURE_LOAD_INFO_T {
    const char *name;
    TextureCompressionType compression_type;

    struct {
        u8 *start;
        i64 len;
    } bytes;
} TextureLoadInfo;

typedef struct TEXTURES_T {
    ARRAY_FIELDS(Texture*)
} Textures;

typedef enum TEXTURE_SAMPLE_FILTER_TYPE_E {
#define X_TEXTURE_SAMPLE_FILTER_TYPES \
    X(NEAREST) \
    X(COUNT)
#define X(type) \
    TEXTURE_SAMPLE_FILTER_TYPE_##type,
    X_TEXTURE_SAMPLE_FILTER_TYPES
#undef X
} TextureSampleFilterMode;

typedef enum TEXTURE_SAMPLE_WRAP_MODE_E {
#define X_TEXTURE_SAMPLE_WRAP_MODES \
    X(CLAMP) \
    X(REPEAT) \
    X(MIRROR) \
    X(COUNT)
#define X(mode) \
    TEXTURE_SAMPLE_WRAP_MODE_##mode,
    X_TEXTURE_SAMPLE_WRAP_MODES
#undef X
} TextureSampleWrapMode;

typedef struct TEXTURE_SAMPLE_INFO_T {
    struct {
        TextureSampleFilterMode min;
        TextureSampleFilterMode mag;
    } filter;

    struct {
        TextureSampleWrapMode horizontal;
        TextureSampleWrapMode vertical;
    } wrap;
} TextureSampleInfo;

Texture *texture_load(const TextureLoadInfo *load_info);

void texture_unload(Texture *texture);

rgba32f texture_sample(const Texture *texture, f32x2 coord, TextureSampleInfo info);

#endif //BRITANNICUS_TEXTURE_H
