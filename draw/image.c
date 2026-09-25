//
// Created by wright on 9/25/26.
//

#include "image.h"

#include "../../debug.h"
#include "../../memory.h"
#include "framebuffer.impl.h"
#include "pixels.h"

#define STBI_MALLOC alt_malloc
#define STBI_REALLOC alt_realloc
#define STBI_FREE alt_free
#define STB_IMAGE_IMPLEMENTATION
#include "../../libs/stb_image.h"

Framebuffer *image_load(const ImageLoadInfo *info) {
    Framebuffer *fb = nullptr;

    switch (info->compression) {
        case IMAGE_COMPRESSION_TYPE_PNG: {
            i32 width, height, channel_count;
            stbi_uc *image_bytes = stbi_load_from_memory(
                info->bytes.data,
                info->bytes.len,
                &width, &height,
                &channel_count,
                4
            );

            if (!image_bytes) {
                crash_msg("Failed to decompress image\n");
            }

            FramebufferCreateInfo fb_create_info = {
                .type = FRAMEBUFFER_TYPE_PIXEL,
                .data = {
                    .pixel_buf = {
                        .format = info->dst_format,
                        .size = {
                            .width = width,
                            .height = height,
                        }
                    }
                }
            };

            fb = framebuffer_create(&fb_create_info);
            FramebufferInfo fb_info = framebuffer_get_info(fb);
            u8 *fb_bytes = framebuffer_impl_get_bytes(fb);
            i32 fb_px_stride = pixel_size(info->dst_format);

            for (i64 y_idx = 0; y_idx < height; y_idx++) {
                for (i64 x_idx = 0; x_idx < width; x_idx++) {
                    u8 *fb_px_start = fb_bytes + (y_idx * fb_info.data.pixel_buf.pitch_bytes) + (x_idx * fb_px_stride);
                    stbi_uc *image_px_start = image_bytes + (y_idx * width * sizeof(argb8)) + (x_idx * sizeof(argb8));

                    Pixel src = {
                        .format = PIXEL_FORMAT_ABGR8,
                        .px_start = image_px_start,
                    };

                    Pixel dst = {
                        .format = fb_info.data.pixel_buf.format,
                        .px_start = fb_px_start,
                    };

                    pixel_set(&dst, &src);
                }
            }

            stbi_image_free(image_bytes);

            break;
        }
        default:
            crash_msg("Unhandled compression type %d", info->compression);
            break;
    }

    return fb;
}
