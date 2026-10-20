//
// Created by wright on 10/16/26.
//

#ifndef BRITANNICUS_SHADER_H
#define BRITANNICUS_SHADER_H

#include "../types.h"
#include "pixels.h"
#include "model.h"

typedef void (*ShaderDataInitFnPtr)(const Model *model, i64 mesh_idx, i64 prim_idx, void *data);

typedef f32x4 (*VertexShaderFnPtr)(i64 indice, void *data);

typedef rgba8 (*FragmentShaderFnPtr)(f32x3 fragment, f32x3 barycentric, i64 triangle[3], void *data);

#endif //BRITANNICUS_SHADER_H
