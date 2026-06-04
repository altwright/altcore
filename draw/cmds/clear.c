//
// Created by wright on 5/8/26.
//

#include "clear.h"

#include <assert.h>
#include "../../memory.h"
#include "../framebuffer_impl.h"

void soft_cmd_clear(
    Framebuffer *px_buf,
    RGBA8888 rgba
) {
    FramebufferInfo px_buf_info = framebuffer_get_info(px_buf);
    assert(px_buf_info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8 *px_buf_bytes = framebuffer_impl_get_bytes(px_buf);
    PixelFormat px_format = px_buf_info.data.pixel_buf.format;
    i32x2 px_buf_size = px_buf_info.data.pixel_buf.size;
    i64 px_buf_pitch_bytes = px_buf_info.data.pixel_buf.pitch_bytes;

    i32 pixel_size = pixels_get_size(px_format);
    PixelColor clear_color = pixels_convert_rgba(px_format, rgba);

    for (i32 y_idx = 0; y_idx < px_buf_size.y; y_idx++) {
        for (i32 x_idx = 0; x_idx < px_buf_size.x; x_idx++) {
            u8 *pixel_start = px_buf_bytes + (y_idx * px_buf_pitch_bytes + x_idx * pixel_size);
            pixels_set(pixel_start, px_format, clear_color);
        }
    }
}
