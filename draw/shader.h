//
// Created by wright on 10/16/26.
//

#ifndef BRITANNICUS_SHADER_H
#define BRITANNICUS_SHADER_H

#include "../types.h"
#include "pixels.h"

typedef enum FRAGMENT_TYPE_E {
#define X_FRAGMENT_TYPES \
    X(SRGB) \
    X(COUNT)
#define X(type) \
    FRAGMENT_TYPE_##type,
    X_FRAGMENT_TYPES
#undef X
} FragmentType;

typedef struct FRAGMENT_T {
    FragmentType type;

    union {
        rgba8 srgb;
    } data;
} Fragment;

typedef f32x3 (*VertexShaderFnPtr)(f32x3 vertex, i64 indice_idx, void *data);

typedef Fragment (*FragmentShaderFnPtr)(f32x3 screen, f32x3 barycentric, i64 start_indice_idx, void *data);

#endif //BRITANNICUS_SHADER_H
