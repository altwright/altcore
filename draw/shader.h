//
// Created by wright on 10/16/26.
//

#ifndef BRITANNICUS_SHADER_H
#define BRITANNICUS_SHADER_H

#include "../types.h"
#include "pixels.h"
#include "model.h"

typedef void (*ShaderDataInitFnPtr)(MeshPrimitive *prim, void *data);

typedef f32x4 (*VertexShaderFnPtr)(f32x3 vertex, i64 indice, void *data);

typedef rgba8 (*FragmentShaderFnPtr)(f32x3 fragment, void *data);

#endif //BRITANNICUS_SHADER_H
