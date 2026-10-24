//
// Created by wright on 10/24/26.
//

#include "clear.h"

#include <string.h>

#include "../framebuffer.impl.h"
#include "../../debug.h"

void soft_cmd_clear(RenderCmdClear *data) {
    FramebufferInfo fb_info = framebuffer_get_info(data->framebuffer);

    u8 *bytes = nullptr;
    i64 pitch = 0;
    framebuffer_impl_get_bytes(data->framebuffer, &bytes, &pitch);

    switch (fb_info.type) {
        case FRAMEBUFFER_TYPE_PIXEL: {
            FramebufferPixelInfo pixel_info = fb_info.data.pixel;
            for (i64 y_idx = 0; y_idx < pixel_info.size.height; y_idx++) {
                for (i64 x_idx = 0; x_idx < pixel_info.size.width; x_idx++) {
                    pixel_set(
                        &(Pixel) {
                            .format = pixel_info.format,
                            .start = bytes + y_idx * pitch + x_idx * pixel_size(pixel_info.format),
                        },
                        &(Pixel) {
                            .format = PIXEL_FORMAT_RGBA32F,
                            .start = (u8*)&RGBA32F_BLACK,
                        }
                    );
                }
            }
            break;
        }
        case FRAMEBUFFER_TYPE_DEPTH: {
            FramebufferDepthInfo depth_info = fb_info.data.depth;
            for (i64 y_idx = 0; y_idx < depth_info.size.height; y_idx++) {
                for (i64 x_idx = 0; x_idx < depth_info.size.width; x_idx++) {
                    auto val = (f32 *) (bytes + y_idx * pitch + x_idx * sizeof(f32));
                    *val = 1.0f;
                }
            }
            break;
        }
        default:
            crash_msg("Unhandled framebuffer type");
            break;
    }
}
