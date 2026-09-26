//
// Created by wright on 5/25/26.
//

#include "draw_text.h"

#include <assert.h>

#include "../fonts.impl.h"
#include "../framebuffer.impl.h"

void soft_cmd_draw_text(
    Framebuffer *px_buf,
    i32x4 px_buf_dst,
    string_view text,
    FontHandle *font,
    i32 font_size_px,
    i32 letter_spacing_px,
    rgba8 color
) {
    FramebufferInfo px_buf_info = framebuffer_get_info(px_buf);
    assert(px_buf_info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8 *px_buf_bytes = framebuffer_impl_get_bytes(px_buf);

    float scale_factor = font_impl_get_scale_factor(font, font_size_px);
    stbtt_fontinfo *font_info = font_impl_get_info(font);

    i32 max_x0, max_x1, max_y0, max_y1;
    font_impl_get_max_bbox(font, &max_x0, &max_y0, &max_x1, &max_y1);

    max_x0 = (i32) (scale_factor * (f32) max_x0);
    max_y0 = (i32) (scale_factor * (f32) max_y0);
    max_x1 = (i32) (scale_factor * (f32) max_x1);
    max_y1 = (i32) (scale_factor * (f32) max_y1);

    i32x2 cursor = {.x = 0, .y = px_buf_dst.height + max_y0};

    for (i32 c_idx = 0; c_idx < text.len; c_idx++) {
        const char *c = &text.start[c_idx];

        i32 glyph_idx = font_impl_get_glyph_idx(font, c);
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
        u8 *bitmap_bytes = nullptr;

        if (!font_impl_codepoint_bitmap_exists(font, c, font_size_px)) {
            font_impl_create_codepoint_bitmap(font, c, font_size_px, bitmap_width, bitmap_height);
            font_impl_get_codepoint_bitmap(font, c, font_size_px, &bitmap_bytes, &bitmap_width, &bitmap_height);

            stbtt_MakeGlyphBitmap(
                font_info,
                bitmap_bytes,
                bitmap_width,
                bitmap_height,
                bitmap_width,
                scale_factor,
                scale_factor,
                glyph_idx
            );
        } else {
            font_impl_get_codepoint_bitmap(font, c, font_size_px, &bitmap_bytes, &bitmap_width, &bitmap_height);
        }

        i32 lsb = (i32) (scale_factor * (float) font_impl_get_left_side_bearing(font, c));
        cursor.x += lsb;

        PixelFormat px_format = px_buf_info.data.pixel_buf.format;
        i32 px_size = (i32)pixel_size(px_format);

        for (i32 bitmap_row_idx = 0; bitmap_row_idx < bitmap_height; bitmap_row_idx++) {
            for (i32 bitmap_col_idx = 0; bitmap_col_idx < bitmap_width; bitmap_col_idx++) {
                u8 bitmap_byte = bitmap_bytes[bitmap_row_idx * bitmap_width + bitmap_col_idx];

                if (bitmap_byte == 0) {
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

                rgba8 final_color = color;
                f32 bitmap_alpha = (f32)bitmap_byte / 255.f;
                f32 color_alpha = (f32)color.a / 255.f;
                final_color.a = (u8)(bitmap_alpha * color_alpha * 255.f);

                Pixel dst = {
                    .format = px_buf_info.data.pixel_buf.format,
                    .start = px_byte_start,
                };

                Pixel src = {
                    .format = PIXEL_FORMAT_RGBA8,
                    .start = (u8*)&final_color,
                };

                pixel_set(&dst, &src);
            }
        }

        i32 advance_width = (i32) (scale_factor * (float) font_impl_get_advance_width(font, c));

        cursor.x += advance_width;
        cursor.x += letter_spacing_px;
    }
}
