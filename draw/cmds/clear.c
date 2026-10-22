//
// Created by wright on 5/8/26.
//

#include "clear.h"

#include <assert.h>
#include "../../memory.h"
#include "../framebuffer.impl.h"

void soft_cmd_clear(
    Framebuffer *px_buf,
    rgba8 rgba
) {
    FramebufferInfo px_buf_info = framebuffer_get_info(px_buf);
    assert(px_buf_info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8 *px_buf_bytes = nullptr;
    i64 px_buf_pitch_bytes = 0;
    framebuffer_impl_get_bytes(px_buf, &px_buf_bytes, &px_buf_pitch_bytes);
    PixelFormat px_format = px_buf_info.data.pixel.format;
    i32x2 px_buf_size = px_buf_info.data.pixel.size;

    i32 px_stride = (i32)pixel_size(px_format);
    Pixel src_px = {
        .format = PIXEL_FORMAT_RGBA8,
        .start = (u8*)&rgba,
    };

    for (i32 y_idx = 0; y_idx < px_buf_size.y; y_idx++) {
        for (i32 x_idx = 0; x_idx < px_buf_size.x; x_idx++) {
            u8 *pixel_start = px_buf_bytes + (y_idx * px_buf_pitch_bytes + x_idx * px_stride);

            Pixel dst_px = {
                .format = px_format,
                .start = pixel_start,
            };

            pixel_set(&dst_px, &src_px);
        }
    }
}
