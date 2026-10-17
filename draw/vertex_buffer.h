//
// Created by wright on 10/16/26.
//

#ifndef BRITANNICUS_VERTEX_BUFFER_H
#define BRITANNICUS_VERTEX_BUFFER_H

#include "../../types.h"

typedef struct VERTEX_BUFFER_T VertexBuffer;
typedef struct RENDERER_T Renderer;

typedef enum VERTEX_COMPONENT_TYPE_E {
#define X_VERTEX_COMPONENT_TYPES \
    X(F32) \
    X(COUNT)
#define X(type) \
    VERTEX_COMPONENT_TYPE_##type,
    X_VERTEX_COMPONENT_TYPES
#undef X
} VertexComponentType;

typedef struct VERTEX_BUFFER_CREATE_INFO_T {
    Renderer *renderer;

    struct {
        VertexComponentType type;
        u8 *start;
        i64 stride_bytes;
        i64 count;
    } vertices;
} VertexBufferCreateInfo;

VertexBuffer *vertex_buffer_create(VertexBufferCreateInfo *info);

void vertex_buffer_destroy(VertexBuffer *buffer);

#endif //BRITANNICUS_VERTEX_BUFFER_H
