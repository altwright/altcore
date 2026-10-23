//
// Created by wright on 5/3/26.
//

#include "framebuffer.h"

#include <assert.h>

#include "../memory.h"
#include "../debug.h"
#include "../arenas.h"
#include "../maths.h"
#include "window.h"
#include "framebuffer.impl.h"

#define STBI_MALLOC alt_malloc
#define STBI_REALLOC alt_realloc
#define STBI_FREE alt_free
#define STB_IMAGE_IMPLEMENTATION
#include "../../libs/stb_image.h"

typedef enum FRAMEBUFFER_PIXEL_COMPRESSION_TYPE_E {
#define X_FRAMEBUFFER_PIXEL_COMPRESSION_TYPES \
    X(NONE) \
    X(COUNT)
#define X(type) \
    FRAMEBUFFER_PIXEL_COMPRESSION_TYPE_##type,
    X_FRAMEBUFFER_PIXEL_COMPRESSION_TYPES
#undef X
} FramebufferPixelCompressionType;

typedef struct FRAMEBUFFER_PIXEL_T {
    Arena *arena;
    PixelFormat read_format;
    i32x2 size;
    f32x4 scissor;
    FramebufferPixelCompressionType type;

    union {
        struct {
            u8s bytes;
            i64 pitch;
        } none;
    } data;
} FramebufferPixel;

struct FRAMEBUFFER_T {
    FramebufferType type;

    union {
        FramebufferPixel pixel;
    } data;
};

Framebuffer *framebuffer_create(const FramebufferCreateInfo *create_info) {
    Framebuffer *fb = alt_malloc(sizeof(*fb));

    *fb = (Framebuffer){
        .type = create_info->type,
    };

    switch (create_info->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixel *pixel_buf = &fb->data.pixel;

            const FramebufferPixelCreateInfo *pixel_info = &create_info->data.pixel;

            switch (pixel_info->type) {
                case FRAMEBUFFER_CREATE_ACTION_TYPE_NEW: {
                    auto new_info = &pixel_info->data.new;

                    i64 px_size = pixel_size(new_info->dst_format);
                    *pixel_buf = (FramebufferPixel){
                        .arena = arena_make(new_info->size.x * new_info->size.y * px_size),
                        .read_format = new_info->dst_format,
                        .size = new_info->size,
                        .scissor = {
                            .start_x = 0,
                            .start_y = 0,
                            .width = (f32) new_info->size.x,
                            .height = (f32) new_info->size.y,
                        },
                        .type = FRAMEBUFFER_PIXEL_COMPRESSION_TYPE_NONE,
                    };

                    auto uncompressed = &pixel_buf->data.none;

                    uncompressed->bytes = (u8s){
                        .arena = pixel_buf->arena,
                        .len = new_info->size.x * new_info->size.y * px_size
                    };
                    ARRAY_MAKE(&uncompressed->bytes);

                    uncompressed->pitch = new_info->size.x * px_size;

                    break;
                }
                case FRAMEBUFFER_CREATE_ACTION_TYPE_LOAD: {
                    auto load_info = &pixel_info->data.load;

                    *pixel_buf = (FramebufferPixel){
                        .read_format = load_info->dst_format,
                        .type = FRAMEBUFFER_PIXEL_COMPRESSION_TYPE_NONE,
                    };

                    switch (load_info->type) {
                        case FRAMEBUFFER_LOAD_COMPRESSION_TYPE_NONE: {
                            auto uncompressed_info = &load_info->data.none;

                            pixel_buf->size = uncompressed_info->size;
                            pixel_buf->scissor = (f32x4){
                                .start_x = 0,
                                .start_y = 0,
                                .width = (f32) uncompressed_info->size.width,
                                .height = (f32) uncompressed_info->size.height,
                            };

                            i64 dst_px_size = pixel_size(load_info->dst_format);
                            pixel_buf->arena = arena_make(pixel_buf->size.width * pixel_buf->size.height * dst_px_size);

                            auto uncompressed_dst = &pixel_buf->data.none;
                            uncompressed_dst->bytes = (u8s) {
                                .arena = pixel_buf->arena,
                                .len = pixel_buf->size.width * pixel_buf->size.height * dst_px_size,
                            };
                            ARRAY_MAKE(&uncompressed_dst->bytes);
                            uncompressed_dst->pitch = pixel_buf->size.width * dst_px_size;

                            i64 src_px_size = pixel_size(uncompressed_info->src_format);

                            for (i64 y_idx = 0; y_idx < pixel_buf->size.height; y_idx++) {
                                for (i64 x_idx = 0; x_idx < pixel_buf->size.width; x_idx++) {
                                    i64 src_px_idx = y_idx * uncompressed_info->pitch + x_idx * src_px_size;
                                    u8 *src_px = uncompressed_info->data + src_px_idx;

                                    i64 dst_px_idx = dst_px_size * (y_idx * pixel_buf->size.width + x_idx);
                                    u8* dst_px = ARRAY_GET(&uncompressed_dst->bytes, dst_px_idx);

                                    pixel_set(
                                        &(Pixel) {
                                            .format = load_info->dst_format,
                                            .start = dst_px,
                                        },
                                        &(Pixel) {
                                            .format = uncompressed_info->src_format,
                                            .start = src_px,
                                        }
                                    );
                                }
                            }

                            break;
                        }
                        case FRAMEBUFFER_LOAD_COMPRESSION_TYPE_PNG:
                        case FRAMEBUFFER_LOAD_COMPRESSION_TYPE_JPEG: {
                            const u8 *compressed_bytes = nullptr;
                            i64 compressed_len = 0;
                            if (load_info->type == FRAMEBUFFER_LOAD_COMPRESSION_TYPE_PNG) {
                                compressed_bytes = load_info->data.png.data;
                                compressed_len = load_info->data.png.len;
                            } else {
                                compressed_bytes = load_info->data.jpeg.data;
                                compressed_len = load_info->data.jpeg.len;
                            }

                            i32 width, height, channel_count;
                            i32 image_px_channels = sizeof(rgba8);
                            stbi_uc *image_bytes = stbi_load_from_memory(
                                compressed_bytes,
                                (i32) compressed_len,
                                &width,
                                &height,
                                &channel_count,
                                image_px_channels
                            );

                            if (!image_bytes) {
                                crash_msg("Failed to decompress image\n");
                            }
                            pixel_buf->size = (i32x2){
                                .width = width,
                                .height = height,
                            };

                            pixel_buf->scissor = (f32x4){
                                .start_x = 0,
                                .start_y = 0,
                                .width = (f32) width,
                                .height = (f32) height,
                            };

                            i64 uncompressed_px_size = pixel_size(load_info->dst_format);
                            pixel_buf->arena = arena_make(width * height * uncompressed_px_size);

                            auto uncompressed = &pixel_buf->data.none;

                            uncompressed->bytes = (u8s){
                                .arena = pixel_buf->arena,
                                .len = width * height * uncompressed_px_size,
                            };
                            ARRAY_MAKE(&uncompressed->bytes);
                            uncompressed->pitch = width * uncompressed_px_size;

                            for (i64 y_idx = 0; y_idx < height; y_idx++) {
                                for (i64 x_idx = 0; x_idx < width; x_idx++) {
                                    i64 uncompressed_px_idx = uncompressed_px_size * (y_idx * width + x_idx);
                                    u8 *uncompressed_px = ARRAY_GET(&uncompressed->bytes, uncompressed_px_idx);

                                    i64 image_px_idx = image_px_channels * (y_idx * width + x_idx);
                                    u8 *image_px = image_bytes + image_px_idx;

                                    pixel_set(
                                        &(Pixel){
                                            .format = load_info->dst_format,
                                            .start = uncompressed_px,
                                        },
                                        &(Pixel){
                                            .format = PIXEL_FORMAT_RGBA8,
                                            .start = image_px,
                                        }
                                    );
                                }
                            }

                            stbi_image_free(image_bytes);
                            break;
                        }
                        default:
                            crash_msg("Unhandled load compression type\n");
                            break;
                    }
                    break;
                }
                default:
                    crash_msg("Unhandled pixel buffer create type\n");
                    break;
            }
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", create_info->type);
            break;
    }

    return fb;
}

void framebuffer_destroy(Framebuffer **fb) {
    switch ((*fb)->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixel *pixel_buf = &(*fb)->data.pixel;
            arena_free(&pixel_buf->arena);
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", (*fb)->type);
            break;
    }

    alt_free(*fb);
    *fb = nullptr;
}

FramebufferInfo framebuffer_get_info(const Framebuffer *fb) {
    FramebufferInfo info = {};

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            const FramebufferPixel *pixel_buf = &fb->data.pixel;

            info.type = FRAMEBUFFER_TYPE_PIXEL;
            FramebufferPixelInfo *pixel_info = &info.data.pixel;
            pixel_info->format = pixel_buf->read_format;
            pixel_info->size = pixel_buf->size;
            pixel_info->scissor = pixel_buf->scissor;

            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }

    return info;
}

void framebuffer_impl_get_bytes(Framebuffer *fb, u8** out_bytes, i64* out_pitch) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixel *pixel_buf = &fb->data.pixel;
            switch (pixel_buf->type) {
                case FRAMEBUFFER_PIXEL_COMPRESSION_TYPE_NONE: {
                    *out_bytes = pixel_buf->data.none.bytes.data;
                    *out_pitch = pixel_buf->data.none.pitch;
                    break;
                }
                default:
                    crash_msg("Unhandled compression type\n");
                    break;
            }
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }
}

void framebuffer_impl_set_scissor(Framebuffer *fb, f32x4 region) {
    if (region.start_x < 0) {
        region.width += region.start_x;
        region.start_x = 0;
    }
    if (region.start_y < 0) {
        region.height += region.start_y;
        region.start_y = 0;
    }
    if (region.width < 0) {
        region.width = 0;
    }
    if (region.height < 0) {
        region.height = 0;
    }

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            fb->data.pixel.scissor = region;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }
}

rgba32f framebuffer_get_sample(Framebuffer *fb, f32x2 normalized_coord, FramebufferSampleInfo sample_info) {
    rgba32f sample = {};

    switch (sample_info.filter) {
        case FRAMEBUFFER_SAMPLE_FILTER_MODE_NEAREST: {

            if (sample_info.wrap.horizontal == FRAMEBUFFER_SAMPLE_WRAP_MODE_CLAMP
                && sample_info.wrap.vertical == FRAMEBUFFER_SAMPLE_WRAP_MODE_CLAMP) {

                normalized_coord.u = SATURATE(normalized_coord.u);
                normalized_coord.v = SATURATE(normalized_coord.v);
            }
            else {
                crash_msg("Unhandled wrap combination\n");
            }

            FramebufferInfo texture_info = framebuffer_get_info(fb);
            u8* bytes = nullptr;
            i64 pitch = 0;
            framebuffer_impl_get_bytes(fb, &bytes, &pitch);

            FramebufferPixelInfo* pixel_buf_info = &texture_info.data.pixel;
            i32 p_x = (i32) (((f32) pixel_buf_info->size.width * normalized_coord.u) + 0.5f);
            i32 p_y = (i32) (((f32) pixel_buf_info->size.height * normalized_coord.v) + 0.5f);

            if (p_x >= pixel_buf_info->size.width) {
                p_x = pixel_buf_info->size.width - 1;
            }

            if (p_y >= pixel_buf_info->size.height) {
                p_y = pixel_buf_info->size.height - 1;
            }

            i64 px_size = pixel_size(pixel_buf_info->format);

            pixel_set(
                &(Pixel){
                    .format = PIXEL_FORMAT_RGBA32F,
                    .start = (u8 *) sample.data,
                },
                &(Pixel){
                    .format = pixel_buf_info->format,
                    .start = bytes + p_y * pitch + p_x * px_size
                }
            );

            break;
        }
        default:
            crash_msg("Unhandled filter mode\n");
            break;
    }



    return sample;
}