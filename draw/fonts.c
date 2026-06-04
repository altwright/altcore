//
// Created by wright on 5/23/26.
//

#include <string.h>
#include <assert.h>

#include "fonts.h"

#include <ctype.h>

#include "../libs/stb_truetype.h"
#include "../memory.h"

constexpr i32 kDefaultUpperFontSizePx = 128;
constexpr i32 kAsciiCodepointCount = 128;

typedef struct CODEPOINT_INFO_T {
    i32 glyph_idx;
    i32 advance_width_units;
    i32 left_side_bearing_units;

    struct {
        i32 x0; // left of current point
        i32 y0; // above baseline
        i32 x1; // right of current point
        i32 y1; // below baseline
    } bbox;

    struct {
        i32 start, end;
    } kerning_entry_idxs;
} CodepointInfo;

struct FONT_HANDLE_T {
    Arena *arena;

    string filepath;
    u8s ttf;
    stbtt_fontinfo info;
    f32s scale_factor_per_px_heights;

    struct {
        // scale_factor * -1 * y0 is the top edge of the glyph relative to the baseline.
        // The character should be displayed in the rectangle from
        // <current_point+SF*x0, baseline+SF*y0> to <current_point+SF*x1,baseline+SF*y1).
        i32 x0; // left of current point
        i32 y0; // above baseline
        i32 x1; // right of current point
        i32 y1; // below baseline
    } max_bbox;

    struct {
        ARRAY_FIELDS(CodepointInfo)
    } ascii_codepoint_infos;

    /*
     * Sorted in ascending glyph1 and glyph2 order
     */
    struct {
        ARRAY_FIELDS(stbtt_kerningentry)
    } kerning_table;

    struct {
        i32 width;
        i32 height;
        u8s bytes;
    } max_bitmap;
};

FontHandle *font_load_ttf(Filepath *path) {
    FileHandle *file = fs_file_open(path, FILE_HANDLE_MODE_BINARY_READ);
    if (!file) {
        return nullptr;
    }

    FontHandle *font = alt_malloc(sizeof(*font));

    font->arena = arena_make(512 * KiB);

    font->ttf = fs_file_to_buf(font->arena, file);

    fs_file_close(file);

    font->filepath = fs_path_get_abs(font->arena, path);

    int success = stbtt_InitFont(&font->info, font->ttf.data, 0);
    if (!success) {
        return nullptr;
    }

    font->scale_factor_per_px_heights.arena = font->arena;
    font->scale_factor_per_px_heights.len = kDefaultUpperFontSizePx;
    ARRAY_MAKE(&font->scale_factor_per_px_heights);

    for (i64 px_idx = 0; px_idx < font->scale_factor_per_px_heights.len; px_idx++) {
        font->scale_factor_per_px_heights.data[px_idx] = F32(
            stbtt_ScaleForPixelHeight(
                &font->info,
                (float) (px_idx + 1)
            )
        );
    }

    stbtt_GetFontBoundingBox(
        &font->info,
        &font->max_bbox.x0,
        &font->max_bbox.y0,
        &font->max_bbox.x1,
        &font->max_bbox.y1
    );

    font->kerning_table.arena = font->arena;
    font->kerning_table.len = stbtt_GetKerningTableLength(&font->info);
    ARRAY_MAKE(&font->kerning_table);

    stbtt_GetKerningTable(&font->info, font->kerning_table.data, (i32) font->kerning_table.len);

    font->ascii_codepoint_infos.arena = font->arena;
    font->ascii_codepoint_infos.len = kAsciiCodepointCount;
    ARRAY_MAKE(&font->ascii_codepoint_infos);

    for (i64 ascii_idx = 0; ascii_idx < kAsciiCodepointCount; ascii_idx++) {
        CodepointInfo *ascii_info = ARRAY_GET(&font->ascii_codepoint_infos, ascii_idx);
        ascii_info->glyph_idx = stbtt_FindGlyphIndex(&font->info, (i32) ascii_idx);
        if (ascii_info->glyph_idx) {
            stbtt_GetGlyphHMetrics(
                &font->info,
                ascii_info->glyph_idx,
                &ascii_info->advance_width_units,
                &ascii_info->left_side_bearing_units
            );

            stbtt_GetGlyphBox(
                &font->info,
                ascii_info->glyph_idx,
                &ascii_info->bbox.x0,
                &ascii_info->bbox.y0,
                &ascii_info->bbox.x1,
                &ascii_info->bbox.y1
            );

            ascii_info->kerning_entry_idxs.start = ascii_info->kerning_entry_idxs.end = -1;

            for (
                i32 kerning_entry_idx = 0;
                kerning_entry_idx < font->kerning_table.len;
                kerning_entry_idx++
            ) {
                stbtt_kerningentry *entry = ARRAY_GET(&font->kerning_table, kerning_entry_idx);

                if (ascii_info->kerning_entry_idxs.start < 0
                    && ascii_info->glyph_idx == entry->glyph1) {
                    ascii_info->kerning_entry_idxs.start = kerning_entry_idx;
                } else if (ascii_info->kerning_entry_idxs.end < 0
                           && ascii_info->glyph_idx != entry->glyph1) {
                    ascii_info->kerning_entry_idxs.end = kerning_entry_idx - 1;
                    break;
                }
            }

            assert(
                ascii_info->kerning_entry_idxs.start >= 0
                && ascii_info->kerning_entry_idxs.end >= ascii_info->kerning_entry_idxs.start
            );
        }
    }

    float max_scale_factor = f32_float(*ARRAY_GET(&font->scale_factor_per_px_heights, kDefaultUpperFontSizePx - 1));
    float max_bbox_width = max_scale_factor * (float)(font->max_bbox.x1 - font->max_bbox.x0);
    float max_bbox_height = max_scale_factor * (float)(font->max_bbox.y1 - font->max_bbox.y0);

    font->max_bitmap.width = (i32)max_bbox_width + 1;
    font->max_bitmap.height = (i32)max_bbox_height + 1;

    font->max_bitmap.bytes = (u8s){font->arena, font->max_bitmap.width * font->max_bitmap.height};
    ARRAY_MAKE(&font->max_bitmap.bytes);

    return font;
}

void font_unload(FontHandle *font) {
    arena_free(font->arena);
    alt_free(font);
}

f32x2 font_measure_text_line(
    FontHandle *font,
    string_view view,
    i32 height_px,
    i32 letter_spacing_px
) {
    f32 scale_factor = *ARRAY_GET(&font->scale_factor_per_px_heights, height_px);
    f32 y1 = f32_mul(scale_factor, F32(font->max_bbox.y1));
    f32 y0 = f32_mul(scale_factor, F32(font->max_bbox.y0));
    f32 height = f32_sub(y1, y0);

    f32 width = F32(0);

    for (i32 c_idx = 0; c_idx < view.len; c_idx++) {
        char c = view.start[c_idx];

        if (isascii(c)
            && ARRAY_GET(&font->ascii_codepoint_infos, c)->glyph_idx
        ) {
            width = f32_add(
                width,
                f32_mul(
                    scale_factor,
                    F32(ARRAY_GET(&font->ascii_codepoint_infos, c)->advance_width_units)
                )
            );

            if (c_idx < view.len - 1) {
                width = f32_add(
                    width,
                    F32(letter_spacing_px)
                );
            }
        }
    }

    f32x2 dim = {
        .width = width,
        .height = height,
    };

    return dim;
}

stbtt_fontinfo *font_impl_get_info(FontHandle *font) {
    return &font->info;
}

float font_impl_get_scale_factor(FontHandle* font, i32 px_size) {
    assert(font && px_size > 0 && px_size <= kDefaultUpperFontSizePx);
    return f32_float(*ARRAY_GET(&font->scale_factor_per_px_heights, px_size - 1));
}

i32 font_impl_get_glyph_idx(FontHandle* font, const char* codepoint) {
    i32 glyph_idx = 0;

    if (isascii(*codepoint)) {
        glyph_idx = ARRAY_GET(&font->ascii_codepoint_infos, *codepoint)->glyph_idx;
    }

    return glyph_idx;
}

i32 font_impl_get_left_side_bearing(FontHandle* font, const char* codepoint) {
    i32 lsb = 0;

    if (isascii(*codepoint)) {
        lsb = ARRAY_GET(&font->ascii_codepoint_infos, *codepoint)->left_side_bearing_units;
    }

    return lsb;
}

i32 font_impl_get_advance_width(FontHandle* font, const char* codepoint) {
    i32 adv_width = 0;

    if (isascii(*codepoint)) {
        adv_width = ARRAY_GET(&font->ascii_codepoint_infos, *codepoint)->advance_width_units;
    }

    return adv_width;
}

void font_impl_get_max_bbox(FontHandle* font, i32* x0, i32* y0, i32* x1, i32* y1) {
    *x0 = font->max_bbox.x0;
    *y0 = font->max_bbox.y0;
    *x1 = font->max_bbox.x1;
    *y1 = font->max_bbox.y1;
}

void font_impl_get_max_bitmap(FontHandle* font, u8** bitmap_bytes, i32* bitmap_width, i32* bitmap_height) {
    *bitmap_bytes = font->max_bitmap.bytes.data;
    *bitmap_width = font->max_bitmap.width;
    *bitmap_height = font->max_bitmap.height;
}
