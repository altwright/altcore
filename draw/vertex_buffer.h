//
// Created by wright on 10/16/26.
//

#ifndef BRITANNICUS_VERTEX_BUFFER_H
#define BRITANNICUS_VERTEX_BUFFER_H

#include "../../types.h"

typedef struct VERTEX_BUFFER_T VertexBuffer;
typedef struct RENDERER_T Renderer;

typedef enum VERTEX_POSITION_COMPONENT_TYPE_E {
#define X_VERTEX_COMPONENT_TYPES \
    X(F32) \
    X(COUNT)
#define X(type) \
    VERTEX_POSITION_COMPONENT_TYPE_##type,
    X_VERTEX_COMPONENT_TYPES
#undef X
} VertexPositionComponentType;

typedef enum VERTEX_ATTRIBUTE_HINT_E {
#define X_VERTEX_ATTRIBUTE_HINTS \
    X(POSITION) \
    X(COLOR) \
    X(NORMAL) \
    X(TEXTURE_COORD) \
    X(TANGENT) \
    X(JOINTS) \
    X(WEIGHTS) \
    X(CUSTOM) \
    X(COUNT)
#define X(hint) \
    VERTEX_ATTRIBUTE_HINT_##hint,
    X_VERTEX_ATTRIBUTE_HINTS
#undef X
} VertexAttributeHint;

typedef enum VERTEX_ATTRIBUTE_TYPE_E {
#define X_VERTEX_ATTRIBUTE_TYPES \
    X(F32X2) \
    X(F32X3) \
    X(F32X4) \
    X(COUNT)
#define X(type) \
    VERTEX_ATTRIBUTE_TYPE_##type,
    X_VERTEX_ATTRIBUTE_TYPES
#undef X
} VertexAttributeType;

typedef struct VERTEX_ATTRIBUTE_T {
    VertexAttributeHint hint;
    VertexAttributeType type;
    u8 *start;
    i64 stride;
} VertexAttribute;

typedef struct VERTEX_ATTRIBUTES_T {
    ARRAY_FIELDS(VertexAttribute)
} VertexAttributes;

typedef struct VERTEX_BUFFER_CREATE_INFO_T {
    Renderer *renderer;

    struct {
        const VertexAttribute *data;
        i64 per_vertex_attribute_count;
        i64 count;
    } vertices;
} VertexBufferCreateInfo;

VertexBuffer *vertex_buffer_create(VertexBufferCreateInfo *info);

void vertex_buffer_destroy(VertexBuffer *buffer);

void vertex_buffer_get_attributes(
    const VertexBuffer *vert_buffer,
    i64 vert_index,
    VertexAttribute *out_attributes,
    i64 *out_attributes_len
);

#endif //BRITANNICUS_VERTEX_BUFFER_H
