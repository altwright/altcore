//
// Created by wright on 5/23/26.
//

#include <string.h>
#include <assert.h>

#include "fonts.h"

#include <ctype.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "../libs/stb_truetype.h"

#include "../memory.h"
#include "../hashmap.h"

typedef struct GLYPH_BITMAP_T {
    i32 width, height;
    u8s bytes;
} GlyphBitmap;

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

    struct {
        HASHMAP_FIELDS(i32, GlyphBitmap)
    } bitmaps; // per px height
} CodepointInfo;

struct FONT_HANDLE_T {
    Arena *arena;

    u8s ttf;
    stbtt_fontinfo info;

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
        HASHMAP_FIELDS(i32, f32)
    } scale_factors; // per px heights

    struct {
        CodepointInfo ascii[128];

        struct {
            HASHMAP_FIELDS(const char*, CodepointInfo)
        } non_ascii; // utf-8 key
    } codepoints;

    /*
     * Sorted in ascending glyph1 and glyph2 order
     */
    struct {
        ARRAY_FIELDS(stbtt_kerningentry)
    } kerning_table;
};

FontHandle *font_load(const FontLoadInfo *info) {
    FontHandle *font = alt_malloc(sizeof(*font));

    *font = (FontHandle){
        .arena = arena_make(MIBIBYTE)
    };

    font->ttf = (u8s){
        .arena = font->arena,
        .len = (i64) info->ttf.len
    };
    ARRAY_MAKE(&font->ttf);

    memcpy(font->ttf.data, info->ttf.data, info->ttf.len);

    int success = stbtt_InitFont(&font->info, font->ttf.data, 0);
    if (!success) {
        return nullptr;
    }

    font->scale_factors.type = HASHMAP_TYPE_NON_STR_KEY;
    font->scale_factors.del_freq = HASHMAP_DEL_FREQ_LOW;
    f32 default_sf = 0;
    HASHMAP_MAKE(&font->scale_factors, &default_sf);

    stbtt_GetFontBoundingBox(
        &font->info,
        &font->max_bbox.x0,
        &font->max_bbox.y0,
        &font->max_bbox.x1,
        &font->max_bbox.y1
    );

    font->kerning_table.len = stbtt_GetKerningTableLength(&font->info);
    if (font->kerning_table.len > 0) {
        font->kerning_table.arena = font->arena;
        ARRAY_MAKE(&font->kerning_table);
        stbtt_GetKerningTable(&font->info, font->kerning_table.data, (i32) font->kerning_table.len);
    }

    for (i64 ascii_idx = 0; ascii_idx <= INT8_MAX; ascii_idx++) {
        CodepointInfo *ascii_info = &font->codepoints.ascii[ascii_idx];
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
        }
    }

    return font;
}

void font_unload(FontHandle *font) {
    arena_free(font->arena);
    alt_free(font);
}

f32x2 font_measure_text(
    FontHandle *font,
    string_view line,
    i32 height_px,
    i32 letter_spacing_px
) {
    f32 scale_factor = HASHMAP_GET(&font->scale_factors, &height_px)->value;
    f32 y1 = scale_factor * (f32) font->max_bbox.y1;
    f32 y0 = scale_factor * (f32) font->max_bbox.y0;
    f32 height = y1 - y0;

    f32 width = 0;

    for (i32 c_idx = 0; c_idx < line.len; c_idx++) {
        char c = line.start[c_idx];

        if (isascii(c)
            && font->codepoints.ascii[c].glyph_idx
        ) {
            width += scale_factor * (f32) font->codepoints.ascii[c].advance_width_units;

            if (c_idx < line.len - 1) {
                width += (f32) letter_spacing_px;
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

float font_impl_get_scale_factor(FontHandle *font, i32 px_height) {
    return HASHMAP_GET(&font->scale_factors, &px_height)->value;
}

i32 font_impl_get_glyph_idx(FontHandle *font, const char *codepoint) {
    i32 glyph_idx = 0;

    if (isascii(*codepoint)) {
        glyph_idx = font->codepoints.ascii[*codepoint].glyph_idx;
    }

    return glyph_idx;
}

i32 font_impl_get_left_side_bearing(FontHandle *font, const char *codepoint) {
    i32 lsb = 0;

    if (isascii(*codepoint)) {
        lsb = font->codepoints.ascii[*codepoint].left_side_bearing_units;
    }

    return lsb;
}

i32 font_impl_get_advance_width(FontHandle *font, const char *codepoint) {
    i32 adv_width = 0;

    if (isascii(*codepoint)) {
        adv_width = font->codepoints.ascii[*codepoint].advance_width_units;
    }

    return adv_width;
}

void font_impl_get_max_bbox(FontHandle *font, i32 *x0, i32 *y0, i32 *x1, i32 *y1) {
    *x0 = font->max_bbox.x0;
    *y0 = font->max_bbox.y0;
    *x1 = font->max_bbox.x1;
    *y1 = font->max_bbox.y1;
}

bool font_impl_codepoint_bitmap_exists(FontHandle *font, const char *codepoint, i32 px_height) {
    bool exists = false;

    if (isascii(*codepoint)) {
        exists = HASHMAP_GET(&font->codepoints.ascii[*codepoint].bitmaps, &px_height);
    }

    return exists;
}

void font_impl_create_codepoint_bitmap(FontHandle *font, const char *codepoint, i32 px_height, i32 bitmap_width,
                                       i32 bitmap_height) {
    if (font_impl_codepoint_bitmap_exists(font, codepoint, px_height)) {
        return;
    }

    GlyphBitmap bitmap = {
        .bytes = {
            .arena = arena_alloc(font->arena, bitmap_width * bitmap_height),
            .len = bitmap_width * bitmap_height,
        },
        .width = bitmap_width,
        .height = bitmap_height,
    };
    ARRAY_MAKE(&bitmap.bytes);

    if (isascii(*codepoint)) {
        HASHMAP_PUT(&font->codepoints.ascii[*codepoint].bitmaps, &px_height, &bitmap);
    }
}

void font_impl_get_codepoint_bitmap(
    FontHandle *font,
    const char *codepoint,
    i32 px_height,
    u8 **out_bitmap_bytes,
    i32 *out_bitmap_width,
    i32 *out_bitmap_height
) {
    GlyphBitmap *glyph_bitmap = nullptr;

    if (isascii(*codepoint)) {
        glyph_bitmap = &HASHMAP_GET(&font->codepoints.ascii[*codepoint].bitmaps, &px_height)->value;
    } else {
        return;
    }

    *out_bitmap_bytes = glyph_bitmap->bytes.data;
    *out_bitmap_width = glyph_bitmap->width;
    *out_bitmap_height = glyph_bitmap->height;
}
