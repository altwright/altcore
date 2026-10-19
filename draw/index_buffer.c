//
// Created by wright on 10/16/26.
//

#include "index_buffer.h"

#include <string.h>

#include "index_buffer.impl.h"
#include "../../memory.h"
#include "../../arenas.h"
#include "../../debug.h"

struct INDEX_BUFFER_T {
    Arena *arena;
    u16s indices;
};

IndexBuffer *index_buffer_create(IndexBufferCreateInfo *info) {
    IndexBuffer *index_buffer = alt_malloc(sizeof(IndexBuffer));

    i64 elem_size = 0;
    switch (info->indices.type) {
        case INDICE_TYPE_U16: {
            elem_size = sizeof(u16);
            break;
        }
        default:
            crash_msg("Unhandled indice type %d\n", info->indices.type);
            break;
    }

    *index_buffer = (IndexBuffer){
        .arena = arena_make(info->indices.count * elem_size)
    };

    index_buffer->indices = (u16s){
        .arena = index_buffer->arena,
        .len = info->indices.count,
    };
    ARRAY_MAKE(&index_buffer->indices);

    for (i64 elem_idx = 0; elem_idx < info->indices.count; elem_idx++) {
        u8 *elem_start = info->indices.start + elem_idx * info->indices.stride_bytes;
        u16 *index = ARRAY_GET(&index_buffer->indices, elem_idx);
        memcpy(index, elem_start, elem_size);
    }

    return index_buffer;
}

void index_buffer_destroy(IndexBuffer *buffer) {
    arena_free(buffer->arena);
    alt_free(buffer);
}

void index_buffer_impl_get_indices(IndexBuffer *index_buffer, const u16 **indices, i64 *indices_len) {
    *indices = index_buffer->indices.data;
    *indices_len = index_buffer->indices.len;
}
