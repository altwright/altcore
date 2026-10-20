//
// Created by wright on 10/16/26.
//

#include "vertex_buffer.h"

#include <string.h>

#include "pixels.h"
#include "../debug.h"
#include "../../arenas.h"
#include "../../memory.h"
#include "../../maths.h"

typedef struct VERTEX_ATTRIBUTE_DATA_T {
    VertexAttributeType type;
    VertexAttributeHint hint;
    u8s bytes;
} VertexAttributeData;

typedef struct VERTEX_ATTRIBUTE_DATA_ARRAY_T {
    ARRAY_FIELDS(VertexAttributeData)
} VertexAttributeDataArray;

struct VERTEX_BUFFER_T {
    Arena *arena;
    VertexAttributeDataArray vert_attr_data_array;
    i64 vert_count;
};

static i64 get_attr_size(VertexAttributeType type) {
    i64 attr_size = 0;
    switch (type) {
        case VERTEX_ATTRIBUTE_TYPE_ABGR8: {
            attr_size = (i64) sizeof(abgr8);
            break;
        }
        case VERTEX_ATTRIBUTE_TYPE_ABGR16: {
            attr_size = (i64) sizeof(abgr16);
            break;
        }
        case VERTEX_ATTRIBUTE_TYPE_F32X2: {
            attr_size = (i64) sizeof(f32x2);
            break;
        }
        case VERTEX_ATTRIBUTE_TYPE_F32X3: {
            attr_size = (i64) sizeof(f32x3);
            break;
        }
        case VERTEX_ATTRIBUTE_TYPE_F32X4: {
            attr_size = (i64) sizeof(f32x4);
            break;
        }
        default:
            crash_msg("Unhandled attr type %d\n", type);
            break;
    }

    return attr_size;
}

VertexBuffer *vertex_buffer_create(VertexBufferCreateInfo *info) {
    VertexBuffer *vert_buffer = alt_malloc(sizeof(VertexBuffer));

    *vert_buffer = (VertexBuffer){
        .arena = arena_make(
            info->vertices.per_vertex_attribute_count * (i64) sizeof(VertexAttributeData)
            + info->vertices.count * info->vertices.per_vertex_attribute_count * (i64) sizeof(f32x4)
        )
    };

    vert_buffer->vert_attr_data_array = (VertexAttributeDataArray){
        .arena = vert_buffer->arena,
        .len = info->vertices.per_vertex_attribute_count,
    };
    ARRAY_MAKE(&vert_buffer->vert_attr_data_array);

    vert_buffer->vert_count = info->vertices.count;

    for (i64 attr_idx = 0; attr_idx < info->vertices.per_vertex_attribute_count; attr_idx++) {
        const VertexAttribute *info_attr = &info->vertices.data[attr_idx];
        VertexAttributeData *vert_attr_data = ARRAY_GET(&vert_buffer->vert_attr_data_array, attr_idx);

        vert_attr_data->type = info_attr->type;
        vert_attr_data->hint = info_attr->hint;

        i64 attr_size = get_attr_size(info_attr->type);

        vert_attr_data->bytes = (u8s){
            .arena = vert_buffer->arena,
            .len = info->vertices.count * attr_size,
        };
        ARRAY_MAKE(&vert_attr_data->bytes);

        for (i64 vert_idx = 0; vert_idx < info->vertices.count; vert_idx++) {
            const u8 *info_vert_attr_start = info_attr->start + vert_idx * info_attr->stride;
            memcpy(ARRAY_GET(&vert_attr_data->bytes, vert_idx * attr_size), info_vert_attr_start, attr_size);
        }
    }

    return vert_buffer;
}

void vertex_buffer_destroy(VertexBuffer *buffer) {
    arena_free(buffer->arena);
    alt_free(buffer);
}

void vertex_buffer_get_attributes(
    const VertexBuffer *vert_buffer,
    i64 vert_index,
    VertexAttribute *out_attributes,
    i64 *out_attributes_len
) {
    if (!out_attributes) {
        *out_attributes_len = vert_buffer->vert_attr_data_array.len;
    } else {
        i64 attr_len = MIN(*out_attributes_len, vert_buffer->vert_attr_data_array.len);

        for (i64 attr_idx = 0; attr_idx < attr_len; attr_idx++) {
            VertexAttribute *out_attr = &out_attributes[attr_idx];
            VertexAttributeData *vert_attr_data = ARRAY_GET(&vert_buffer->vert_attr_data_array, attr_idx);

            out_attr->type = vert_attr_data->type;
            out_attr->hint = vert_attr_data->hint;

            i64 attr_size = get_attr_size(vert_attr_data->type);
            out_attr->start = ARRAY_GET(&vert_attr_data->bytes, vert_index * attr_size);
            out_attr->stride = attr_size;
        }
    }
}
