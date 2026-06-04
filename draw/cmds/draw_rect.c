//
// Created by wright on 5/25/26.
//

#include "draw_rect.h"

#include <assert.h>

#include "../../maths.h"
#include "../framebuffer_impl.h"

void soft_cmd_draw_rect(
    Framebuffer* px_buf,
    i32x4 px_buf_dst,
    RGBA8888 bg_color,
    RectCornerRadii corner_radii,
    RGBA8888 border_color,
    RectBorderWidths border_widths
) {
    FramebufferInfo px_buf_info = framebuffer_get_info(px_buf);
    assert(px_buf_info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8* px_buf_bytes = framebuffer_impl_get_bytes(px_buf);
    PixelFormat px_format = px_buf_info.data.pixel_buf.format;
    i32x2 px_buf_size = px_buf_info.data.pixel_buf.size;
    i64 px_buf_pitch_bytes = px_buf_info.data.pixel_buf.pitch_bytes;

    PixelColor bg_px = pixels_convert_rgba(px_format, bg_color);
    i32 px_size = pixels_get_size(px_format);

    i32 start_y = CLAMP(px_buf_dst.start_y, 0, px_buf_size.height);
    i32 end_y = CLAMP(px_buf_dst.start_y + px_buf_dst.height, start_y, px_buf_size.height);
    i32 start_x = CLAMP(px_buf_dst.start_x, 0, px_buf_size.width);
    i32 end_x = CLAMP(px_buf_dst.start_x + px_buf_dst.width, start_x, px_buf_size.width);

    for (i32 y_idx = start_y; y_idx < end_y; y_idx++) {
        for (i32 x_idx = start_x; x_idx < end_x; x_idx++) {
            u8* pixel_start = px_buf_bytes + y_idx * px_buf_pitch_bytes + x_idx * px_size;
            pixels_set(pixel_start, px_format, bg_px);
        }
    }
}
