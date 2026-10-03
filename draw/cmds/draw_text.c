//
// Created by wright on 5/25/26.
//

#include "draw_text.h"

#include <assert.h>
#include <string.h>

#include "../fonts.impl.h"
#include "../framebuffer.impl.h"
#include "../../debug.h"
#include "../../maths.h"

static void draw_text_line(
    FontHandle *font,
    string_view text,
    i32 font_height_px,
    i32 letter_spacing_px,
    rgba8 text_color,
    Framebuffer *dst_fb,
    i32x4 dst_fb_region,
    i32x2 *cursor
) {
    FramebufferInfo dst_fb_info = framebuffer_get_info(dst_fb);
    u8 *dst_fb_bytes = framebuffer_impl_get_bytes(dst_fb);
    stbtt_fontinfo *font_info = font_impl_get_info(font);
    f32 scale_factor = font_impl_get_scale_factor(font, font_height_px);

    const char *prev_utf8 = nullptr;
    STRING_VIEW_FOR(utf8, &text) {
        i32 glyph_idx = font_impl_get_glyph_idx(font, utf8);
        if (glyph_idx > 0) {
            i32 kerning_advance_px = (i32) (scale_factor * (f32) font_impl_get_kerning_advance(font, prev_utf8, utf8));
            cursor->x += kerning_advance_px;

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

            if (!font_impl_codepoint_bitmap_exists(font, utf8, font_height_px)) {
                font_impl_create_codepoint_bitmap(font, utf8, font_height_px, bitmap_width, bitmap_height);
                font_impl_get_codepoint_bitmap(font, utf8, font_height_px, &bitmap_bytes, &bitmap_width,
                                               &bitmap_height);

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
                font_impl_get_codepoint_bitmap(font, utf8, font_height_px, &bitmap_bytes, &bitmap_width,
                                               &bitmap_height);
            }

            PixelFormat px_format = dst_fb_info.data.pixel_buf.format;
            i32 px_size = (i32) pixel_size(px_format);

            for (i32 bitmap_row_idx = 0; bitmap_row_idx < bitmap_height; bitmap_row_idx++) {
                for (i32 bitmap_col_idx = 0; bitmap_col_idx < bitmap_width; bitmap_col_idx++) {
                    u8 bitmap_byte = bitmap_bytes[bitmap_row_idx * bitmap_width + bitmap_col_idx];

                    if (bitmap_byte == 0) {
                        continue;
                    }

                    i32 dst_y_idx = dst_fb_region.y + (cursor->y + y0 + bitmap_row_idx);
                    i32 dst_x_idx = dst_fb_region.x + (cursor->x + x0 + bitmap_col_idx);

                    if (dst_y_idx >= dst_fb_region.y + dst_fb_region.height
                        || dst_x_idx >= dst_fb_region.x + dst_fb_region.width) {
                        continue;
                    }

                    u8 *px_start =
                            dst_fb_bytes + (dst_y_idx * dst_fb_info.data.pixel_buf.pitch_bytes + dst_x_idx * px_size);

                    rgba8 final_color = text_color;
                    f32 bitmap_alpha = (f32) bitmap_byte / 255.f;
                    f32 color_alpha = (f32) text_color.a / 255.f;
                    final_color.a = (u8) (bitmap_alpha * color_alpha * 255.f);

                    Pixel dst = {
                        .format = dst_fb_info.data.pixel_buf.format,
                        .start = px_start,
                    };

                    Pixel src = {
                        .format = PIXEL_FORMAT_RGBA8,
                        .start = (u8 *) &final_color,
                    };

                    pixel_set(&dst, &src);
                }
            }

            i32 advance_width_px = (i32) (scale_factor * (f32) font_impl_get_advance_width(font, utf8));

            cursor->x += advance_width_px + letter_spacing_px;
        }

        prev_utf8 = utf8;
    }
}

f32x2 draw_text_impl_get_start_cursor(
    string_view text,
    FontSets *font_sets,
    FontStyle font_style,
    i32 font_height_px,
    f32 row_height
) {
    f32x2 cursor = {.x = 0, .y = row_height};

    STRING_VIEW_FOR(utf8, &text) {
        i64 current_font_set_idx = 0;
        FontHandle *font = ARRAY_GET(font_sets, current_font_set_idx)->styles[font_style];
        while (font_impl_get_glyph_idx(font, utf8) <= 0) {
            current_font_set_idx++;
            if (current_font_set_idx >= font_sets->len) {
                break;
            }
            font = ARRAY_GET(font_sets, current_font_set_idx)->styles[font_style];
        }

        f32 scale_factor = font_impl_get_scale_factor(font, font_height_px);

        i32 max_x0, max_x1, max_y0, max_y1;
        font_impl_get_max_bbox(font, &max_x0, &max_y0, &max_x1, &max_y1);

        f32 max_y0_px = scale_factor * (f32) max_y0;
        cursor.y = MIN(cursor.y, row_height + max_y0_px);
    }

    return cursor;
}

void soft_cmd_draw_text(RenderCmdDrawText *data) {
    FramebufferInfo px_buf_info = framebuffer_get_info(data->dst_fb);
    if (px_buf_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Expected pixel framebuffer\n");
    }

    i32x2 cursor = ftoi32x2(
        draw_text_impl_get_start_cursor(
            data->text,
            data->font_sets,
            data->font_style,
            data->font_height_px,
            data->dst_fb_region.height));

    const char *prev_text_start = data->text.start;
    i64 prev_font_set_idx = 0;
    STRING_VIEW_FOR(utf8, &data->text) {
        i64 current_font_set_idx = 0;

        FontHandle *font = ARRAY_GET(data->font_sets, current_font_set_idx)->styles[data->font_style];
        while (font_impl_get_glyph_idx(font, utf8) <= 0) {
            current_font_set_idx++;
            if (current_font_set_idx >= data->font_sets->len) {
                current_font_set_idx = 0;
                break;
            }
            font = ARRAY_GET(data->font_sets, current_font_set_idx)->styles[data->font_style];
        }

        if (current_font_set_idx != prev_font_set_idx) {
            draw_text_line(
                font,
                (string_view){
                    .start = prev_text_start,
                    .len = utf8 - prev_text_start,
                },
                data->font_height_px,
                data->letter_spacing_px,
                data->text_color,
                data->dst_fb,
                ftoi32x4(data->dst_fb_region),
                &cursor
            );

            prev_text_start = utf8;
            prev_font_set_idx = current_font_set_idx;
        }
    }

    draw_text_line(
        ARRAY_GET(data->font_sets, prev_font_set_idx)->styles[data->font_style],
        (string_view){
            .start = prev_text_start,
            .len = (data->text.start + data->text.len) - prev_text_start
        },
        data->font_height_px,
        data->letter_spacing_px,
        data->text_color,
        data->dst_fb,
        ftoi32x4(data->dst_fb_region),
        &cursor
    );
}
