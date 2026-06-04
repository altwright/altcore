//
// Created by wright on 5/26/26.
//

#ifndef ALTCORE_FONTS_IMPL_H
#define ALTCORE_FONTS_IMPL_H

#include "fonts.h"
#include "../../libs/stb_truetype.h"

stbtt_fontinfo* font_impl_get_info(FontHandle* font);

float font_impl_get_scale_factor(FontHandle* font, i32 px_size);

i32 font_impl_get_glyph_idx(FontHandle* font, const char* codepoint);

i32 font_impl_get_left_side_bearing(FontHandle* font, const char* codepoint);

i32 font_impl_get_advance_width(FontHandle* font, const char* codepoint);

void font_impl_get_max_bbox(FontHandle* font, i32* x0, i32* y0, i32* x1, i32* y1);

void font_impl_get_max_bitmap(FontHandle* font, u8** bitmap_bytes, i32* bitmap_width, i32* bitmap_height);

#endif //ALTCORE_FONTS_IMPL_H
