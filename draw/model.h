//
// Created by wright on 10/17/26.
//

#ifndef BRITANNICUS_MODEL_H
#define BRITANNICUS_MODEL_H

#include "renderer.h"
#include "vertex_buffer.h"
#include "index_buffer.h"

typedef enum MESH_PRIMITIVE_TYPE_E {
#define X_MESH_PRIMITIVE_TYPES \
    X(TRIANGLES) \
    X(COUNT)
#define X(type) \
    MESH_PRIMITIVE_TYPE_##type,
    X_MESH_PRIMITIVE_TYPES
#undef X
} MeshPrimitiveType;

typedef struct MESH_PRIMITIVE_T {
    MeshPrimitiveType type;
    VertexBuffer *vertices;
    IndexBuffer *indices;
} MeshPrimitive;

typedef struct MESH_PRIMITIVES_T {
    ARRAY_FIELDS(MeshPrimitive)
} MeshPrimitives;

typedef struct MESH_T {
    string name;
    i64 parent_idx;
    MeshPrimitives prims;

    struct {
        f32x3 translation;
        f32x4 rotation;
        f32x3 scale;
    } local;
} Mesh;

typedef struct MESHES_T {
    ARRAY_FIELDS(Mesh)
} Meshes;

typedef struct MODEL_T {
    Arena *arena;
    Meshes meshes;
} Model;

typedef enum MODEL_DATA_TYPE_E {
#define X_MODEL_DATA_TYPES \
    X(GLB) \
    X(COUNT)
#define X(type) \
    MODEL_DATA_TYPE_##type,
    X_MODEL_DATA_TYPES
#undef X
} ModelDataType;

typedef struct MODEL_LOAD_INFO_T {
    Renderer *renderer;
    ModelDataType type;

    struct {
        u8 *bytes;
        i64 len;
    } data;
} ModelLoadInfo;

Model model_load(ModelLoadInfo *info);

void model_unload(Model *model);

#endif //BRITANNICUS_MODEL_H
