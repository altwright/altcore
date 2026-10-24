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

typedef enum FRAMEBUFFER_COMPRESSION_TYPE_E {
#define X_FRAMEBUFFER_COMPRESSION_TYPES \
    X(NONE) \
    X(COUNT)
#define X(type) \
    FRAMEBUFFER_COMPRESSION_TYPE_##type,
    X_FRAMEBUFFER_COMPRESSION_TYPES
#undef X
} FramebufferCompressionType;

typedef struct FRAMEBUFFER_PIXEL_T {
    Arena *arena;
    PixelFormat read_format;
    i32x2 size;
    f32x4 scissor;
    f32x4 viewport;
    FramebufferCompressionType type;

    union {
        struct {
            u8s bytes;
            i64 pitch;
        } none;
    } data;
} FramebufferPixel;

typedef struct FRAMEBUFFER_DEPTH_T {
    Arena *arena;
    i32x2 size;
    f32x4 scissor;
    f32x4 viewport;

    FramebufferCompressionType type;

    union {
        struct {
            f32s vals;
            i64 pitch;
        } none;
    } data;
} FramebufferDepth;

struct FRAMEBUFFER_T {
    FramebufferType type;

    union {
        FramebufferPixel pixel;
        FramebufferDepth depth;
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
                        .type = FRAMEBUFFER_COMPRESSION_TYPE_NONE,
                    };
                    pixel_buf->viewport = pixel_buf->viewport;

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
                        .type = FRAMEBUFFER_COMPRESSION_TYPE_NONE,
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
                            uncompressed_dst->bytes = (u8s){
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
                                    u8 *dst_px = ARRAY_GET(&uncompressed_dst->bytes, dst_px_idx);

                                    pixel_set(
                                        &(Pixel){
                                            .format = load_info->dst_format,
                                            .start = dst_px,
                                        },
                                        &(Pixel){
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
        case FRAMEBUFFER_TYPE_DEPTH: {
            FramebufferDepth *depth_buf = &fb->data.depth;
            const FramebufferDepthCreateInfo *depth_info = &create_info->data.depth;

            switch (depth_info->type) {
                case FRAMEBUFFER_CREATE_ACTION_TYPE_NEW: {
                    depth_buf->type = FRAMEBUFFER_COMPRESSION_TYPE_NONE;
                    auto new_info = &depth_info->data.new;
                    auto uncompressed = &depth_buf->data.none;

                    depth_buf->size = new_info->size;
                    depth_buf->arena = arena_make((i64) sizeof(f32) * depth_buf->size.width * depth_buf->size.height);
                    depth_buf->scissor = (f32x4){
                        .start_x = 0,
                        .start_y = 0,
                        .width = (f32) depth_buf->size.width,
                        .height = (f32) depth_buf->size.height,
                    };
                    depth_buf->viewport = depth_buf->scissor;

                    uncompressed->vals = (f32s){
                        .arena = depth_buf->arena,
                        .len = depth_buf->size.width * depth_buf->size.height,
                    };
                    ARRAY_MAKE(&uncompressed->vals);
                    uncompressed->pitch = (i64) sizeof(f32) * depth_buf->size.width;

                    break;
                }
                default:
                    crash_msg("Unhandled depth buffer create action\n");
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
        case FRAMEBUFFER_TYPE_DEPTH: {
            FramebufferDepth *depth_buf = &(*fb)->data.depth;
            arena_free(&depth_buf->arena);
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
    FramebufferInfo info = {
        .type = fb->type,
    };

    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            const FramebufferPixel *pixel_buf = &fb->data.pixel;
            FramebufferPixelInfo *pixel_info = &info.data.pixel;
            pixel_info->format = pixel_buf->read_format;
            pixel_info->size = pixel_buf->size;
            pixel_info->scissor = pixel_buf->scissor;
            pixel_info->viewport = pixel_buf->viewport;
            break;
        }
        case FRAMEBUFFER_TYPE_DEPTH: {
            const FramebufferDepth *depth_buf = &fb->data.depth;
            FramebufferDepthInfo *depth_info = &info.data.depth;
            depth_info->size = depth_buf->size;
            depth_info->scissor = depth_buf->scissor;
            depth_info->viewport = depth_buf->viewport;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }

    return info;
}

void framebuffer_impl_get_bytes(Framebuffer *fb, u8 **out_bytes, i64 *out_pitch) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixel *pixel_buf = &fb->data.pixel;
            switch (pixel_buf->type) {
                case FRAMEBUFFER_COMPRESSION_TYPE_NONE: {
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
        case FRAMEBUFFER_TYPE_DEPTH: {
            FramebufferDepth *depth_buf = &fb->data.depth;
            switch (depth_buf->type) {
                case FRAMEBUFFER_COMPRESSION_TYPE_NONE: {
                    *out_bytes = (u8 *) depth_buf->data.none.vals.data;
                    *out_pitch = depth_buf->data.none.pitch;
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

void framebuffer_impl_set_viewport(Framebuffer *fb, f32x4 region) {
    switch (fb->type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            fb->data.pixel.viewport = region;
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb->type);
            break;
    }
}

static f32x2 transform_uv(f32x2 normalized_coord, FramebufferInfo fb_info, FramebufferSampleInfo sample_info) {
    if (sample_info.wrap.horizontal == FRAMEBUFFER_SAMPLE_WRAP_MODE_CLAMP
        && sample_info.wrap.vertical == FRAMEBUFFER_SAMPLE_WRAP_MODE_CLAMP) {
        normalized_coord.u = SATURATE(normalized_coord.u);
        normalized_coord.v = SATURATE(normalized_coord.v);
    } else {
        crash_msg("Unhandled wrap combination\n");
    }
    return normalized_coord;
}

static i32x2 to_nearest(f32x2 normalized_coord, i32x2 size) {
    i32 p_x = (i32) (((f32) size.width * normalized_coord.u) + 0.5f);
    i32 p_y = (i32) (((f32) size.height * normalized_coord.v) + 0.5f);

    if (p_x >= size.width) {
        p_x = size.width - 1;
    }

    if (p_y >= size.height) {
        p_y = size.height - 1;
    }

    return (i32x2){
        .x = p_x,
        .y = p_y,
    };
}

rgba32f framebuffer_get_sample(Framebuffer *fb, f32x2 normalized_coord, FramebufferSampleInfo sample_info) {
    rgba32f sample = {};

    FramebufferInfo fb_info = framebuffer_get_info(fb);
    normalized_coord = transform_uv(normalized_coord, fb_info, sample_info);

    switch (fb_info.type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixelInfo *pixel_buf_info = &fb_info.data.pixel;
            u8 *bytes = nullptr;
            i64 pitch = 0;
            framebuffer_impl_get_bytes(fb, &bytes, &pitch);

            switch (sample_info.filter) {
                case FRAMEBUFFER_SAMPLE_FILTER_MODE_NEAREST: {
                    i32x2 px = to_nearest(normalized_coord, pixel_buf_info->size);
                    i64 px_size = pixel_size(pixel_buf_info->format);

                    pixel_set(
                        &(Pixel){
                            .format = PIXEL_FORMAT_RGBA32F,
                            .start = (u8 *) sample.data,
                        },
                        &(Pixel){
                            .format = pixel_buf_info->format,
                            .start = bytes + px.y * pitch + px.x * px_size
                        }
                    );

                    break;
                }
                default:
                    crash_msg("Unhandled filter mode\n");
                    break;
            }

            break;
        }
        case FRAMEBUFFER_TYPE_DEPTH: {
            FramebufferDepthInfo *depth_buf_info = &fb_info.data.depth;
            u8 *bytes = nullptr;
            i64 pitch = 0;
            framebuffer_impl_get_bytes(fb, &bytes, &pitch);

            switch (sample_info.filter) {
                case FRAMEBUFFER_SAMPLE_FILTER_MODE_NEAREST: {
                    i32x2 px = to_nearest(normalized_coord, depth_buf_info->size);

                    sample.r = *(f32 *) (bytes + px.y * pitch + px.x * sizeof(f32));

                    break;
                }
                default:
                    crash_msg("Unhandled filter mode\n");
                    break;
            }

            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb_info.type);
            break;
    }


    return sample;
}

void framebuffer_set_sample(Framebuffer *fb,
                            f32x2 normalized_coord,
                            FramebufferSampleInfo sample_info,
                            rgba32f sample) {
    FramebufferInfo fb_info = framebuffer_get_info(fb);
    normalized_coord = transform_uv(normalized_coord, fb_info, sample_info);

    switch (fb_info.type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixelInfo *pixel_buf_info = &fb_info.data.pixel;
            u8 *bytes = nullptr;
            i64 pitch = 0;
            framebuffer_impl_get_bytes(fb, &bytes, &pitch);

            switch (sample_info.filter) {
                case FRAMEBUFFER_SAMPLE_FILTER_MODE_NEAREST: {
                    i32x2 px = to_nearest(normalized_coord, pixel_buf_info->size);

                    i64 px_size = pixel_size(pixel_buf_info->format);

                    pixel_set(
                        &(Pixel){
                            .format = pixel_buf_info->format,
                            .start = bytes + px.y * pitch + px.x * px_size
                        },
                        &(Pixel){
                            .format = PIXEL_FORMAT_RGBA32F,
                            .start = (u8 *) sample.data,
                        }
                    );

                    break;
                }
                default:
                    crash_msg("Unhandled filter mode\n");
                    break;
            }

            break;
        }
        case FRAMEBUFFER_TYPE_DEPTH: {
            FramebufferDepthInfo *depth_buf_info = &fb_info.data.depth;
            u8 *bytes = nullptr;
            i64 pitch = 0;
            framebuffer_impl_get_bytes(fb, &bytes, &pitch);

            switch (sample_info.filter) {
                case FRAMEBUFFER_SAMPLE_FILTER_MODE_NEAREST: {
                    i32x2 px = to_nearest(normalized_coord, depth_buf_info->size);

                    auto val = (f32 *) (bytes + px.y * pitch + px.x * sizeof(f32));
                    *val = sample.r;

                    break;
                }
                default:
                    crash_msg("Unhandled filter mode\n");
                    break;
            }

            break;
        }
        default:
            crash_msg("Unhandled framebuffer type %d\n", fb_info.type);
            break;
    }
}
