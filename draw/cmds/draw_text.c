//
// Created by wright on 5/25/26.
//

#include "draw_text.h"

#include <assert.h>

#include "../fonts_impl.h"
#include "../framebuffer_impl.h"

void soft_cmd_draw_text(
    Framebuffer *px_buf,
    i32x4 px_buf_dst,
    string_view text,
    FontHandle *font,
    i32 font_size_px,
    i32 letter_spacing_px,
    RGBA8888 color
) {
    FramebufferInfo px_buf_info = framebuffer_get_info(px_buf);
    assert(px_buf_info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8 *px_buf_bytes = framebuffer_impl_get_bytes(px_buf);

    float scale_factor = font_impl_get_scale_factor(font, font_size_px);
    stbtt_fontinfo *font_info = font_impl_get_info(font);

    i32 max_x0, max_x1, max_y0, max_y1;
    font_impl_get_max_bbox(font, &max_x0, &max_y0, &max_x1, &max_y1);

    max_y0 = (i32) (scale_factor * (float) max_y0);

    i32x2 cursor = {.x = 0, .y = -max_y0};

    u8 *max_bitmap_bytes;
    i32 max_bitmap_width, max_bitmap_height;

    font_impl_get_max_bitmap(font, &max_bitmap_bytes, &max_bitmap_width, &max_bitmap_height);

    for (i32 c_idx = 0; c_idx < text.len; c_idx++) {
        char c = text.start[c_idx];

        i32 glyph_idx = font_impl_get_glyph_idx(font, &c);
        if (glyph_idx <= 0) {
            continue;
        }

        i32 x0, x1, y0, y1;
        stbtt_GetGlyphBitmapBox(
            font_info,
            glyph_idx,
            scale_factor,
            scale_factor,
            &x0,
            &y0,
            &x1,
            &y1
        );

        i32 bitmap_width = x1 - x0;
        i32 bitmap_height = y1 - y0;

        i32 lsb = (i32) (scale_factor * (float) font_impl_get_left_side_bearing(font, &c));

        cursor.x += lsb;

        stbtt_MakeGlyphBitmap(
            font_info,
            max_bitmap_bytes,
            bitmap_width,
            bitmap_height,
            max_bitmap_width,
            scale_factor,
            scale_factor,
            glyph_idx
        );

        PixelFormat px_format = px_buf_info.data.pixel_buf.format;
        i32 px_size = pixels_get_size(px_format);

        for (i32 bitmap_row_idx = 0; bitmap_row_idx < bitmap_height; bitmap_row_idx++) {
            for (i32 bitmap_col_idx = 0; bitmap_col_idx < bitmap_width; bitmap_col_idx++) {
                u8 bitmap_byte = max_bitmap_bytes[bitmap_row_idx * max_bitmap_width + bitmap_col_idx];
                if (!bitmap_byte) {
                    continue;
                }

                u8 *px_byte_start = px_buf_bytes
                                    + (
                                        (
                                            (px_buf_dst.y + (cursor.y + y0 + bitmap_row_idx))
                                            * px_buf_info.data.pixel_buf.pitch_bytes
                                        )
                                        + (px_buf_dst.x + (cursor.x + x0 + bitmap_col_idx)) * px_size
                                    );

                RGBA8888 final_color = color;
                final_color.a = bitmap_byte;

                PixelColor px_color = pixels_convert_rgba(px_format, final_color);
                pixels_set(px_byte_start, px_format, px_color);
            }
        }

        i32 advance_width = (i32) (scale_factor * (float) font_impl_get_advance_width(font, &c));

        cursor.x += advance_width;
        cursor.x += letter_spacing_px;
    }
}
