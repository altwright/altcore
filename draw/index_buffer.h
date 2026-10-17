//
// Created by wright on 10/16/26.
//

#ifndef BRITANNICUS_INDEX_BUFFER_H
#define BRITANNICUS_INDEX_BUFFER_H

#include "../../types.h"

typedef struct INDEX_BUFFER_T IndexBuffer;
typedef struct RENDERER_T Renderer;

typedef enum INDICE_TYPE_E {
#define X_INDICE_TYPES \
    X(U16) \
    X(U32) \
    X(COUNT)
#define X(type) \
    INDICE_TYPE_##type,
    X_INDICE_TYPES
#undef X
} IndiceType;

typedef struct INDEX_BUFFER_CREATE_INFO_T {
    Renderer *renderer;

    struct {
        IndiceType type;
        u8 *start;
        i64 stride_bytes;
        i64 count;
    } indices;
} IndexBufferCreateInfo;

IndexBuffer *index_buffer_create(IndexBufferCreateInfo *info);

void index_buffer_destroy(IndexBuffer *buffer);

#endif //BRITANNICUS_INDEX_BUFFER_H
