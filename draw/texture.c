//
// Created by wright on 10/21/26.
//

#include "texture.h"

#include <string.h>

#include "pixels.h"
#include "../debug.h"
#include "../memory.h"
#include "../arenas.h"
#include "../maths.h"
#include "../../libs/stb_image.h"

struct TEXTURE_T {
    Arena *arena;
    bool is_compressed;

    union {
        struct {
            u8s bytes;
            PixelFormat format;
            i32x2 size;
        } uncompressed;

        struct {
            TextureCompressionType type;
        } compressed;
    } data;
};

Texture *texture_load(const TextureLoadInfo *load_info) {
    Texture *texture = alt_malloc(sizeof(Texture));

    switch (load_info->compression_type) {
        case TEXTURE_COMPRESSION_TYPE_PNG: {
            i32 width, height, channel_count;
            stbi_uc *image_bytes = stbi_load_from_memory(
                load_info->bytes.start,
                (i32) load_info->bytes.len,
                &width,
                &height,
                &channel_count,
                4
            );

            if (!image_bytes) {
                crash_msg("Failed to decompress texture\n");
            }

            i64 image_bytes_size = width * height * 4;
            texture->arena = arena_make(image_bytes_size);
            texture->data.uncompressed.bytes = (u8s){
                .arena = texture->arena,
                .len = image_bytes_size,
            };
            ARRAY_MAKE(&texture->data.uncompressed.bytes);

            memcpy(texture->data.uncompressed.bytes.data, image_bytes, image_bytes_size);

            texture->data.uncompressed.format = PIXEL_FORMAT_ABGR8;
            texture->data.uncompressed.size = (i32x2){.width = width, .height = height};

            stbi_image_free(image_bytes);

            break;
        }
        default:
            crash_msg("Unhandled compression type %d\n", load_info->compression_type);
            break;
    }

    return texture;
}

void texture_unload(Texture *texture) {
    arena_free(texture->arena);
    alt_free(texture);
}

rgba32f texture_sample(const Texture *texture, f32x2 coord, TextureSampleInfo info) {
    rgba32f sample = {};

    if (!texture->is_compressed) {
        coord.u = SATURATE(coord.u);
        coord.v = SATURATE(coord.v);

        auto uncompressed = &texture->data.uncompressed;
        i32 p_x = (i32) (((f32) uncompressed->size.x * coord.u) + 0.5f);
        i32 p_y = (i32) (((f32) uncompressed->size.y * coord.v) + 0.5f);

        i64 px_size = pixel_size(uncompressed->format);

        pixel_set(
            &(Pixel){
                .format = PIXEL_FORMAT_RGBA32F,
                .start = (u8 *) sample.data,
            },
            &(Pixel){
                .format = uncompressed->format,
                .start = uncompressed->bytes.data + p_y * uncompressed->size.width + p_x * px_size
            }
        );
    } else {
        crash_msg("Texture is compressed\n");
    }

    return sample;
}
