//
// Created by wright on 10/16/26.
//

#include "vertex_buffer.h"

#include <string.h>

#include "vertex_buffer.impl.h"
#include "../debug.h"
#include "../../arenas.h"
#include "../../memory.h"

struct VERTEX_BUFFER_T {
    Arena *arena;
    f32x3s vertices;
};

VertexBuffer *vertex_buffer_create(VertexBufferCreateInfo *info) {
    VertexBuffer *vert_buffer = alt_malloc(sizeof(VertexBuffer));

    *vert_buffer = (VertexBuffer){
        .arena = arena_make(info->vertices.count * (i64) sizeof(f32x3))
    };

    vert_buffer->vertices = (f32x3s){
        .arena = vert_buffer->arena,
        .len = info->vertices.count,
    };
    ARRAY_MAKE(&vert_buffer->vertices);

    for (i64 vert_idx = 0; vert_idx < info->vertices.count; vert_idx++) {
        u8 *input_vertex_start = info->vertices.start + vert_idx * info->vertices.stride_bytes;
        switch (info->vertices.type) {
            case VERTEX_COMPONENT_TYPE_F32: {
                f32x3 *vertex = ARRAY_GET(&vert_buffer->vertices, vert_idx);
                memcpy(vertex->data, input_vertex_start, sizeof(f32) * 3);
                break;
            }
            default:
                crash_msg("Unhandled vertex component type %d\n", info->vertices.type);
                break;
        }
    }

    return vert_buffer;
}

void vertex_buffer_destroy(VertexBuffer *buffer) {
    arena_free(buffer->arena);
    alt_free(buffer);
}

void vertex_buffer_impl_get_verts(VertexBuffer *vert_buf, const f32x3 **out_verts, i64 *out_verts_len) {
    *out_verts = vert_buf->vertices.data;
    *out_verts_len = vert_buf->vertices.len;
}
