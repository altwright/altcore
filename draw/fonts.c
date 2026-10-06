//
// Created by wright on 5/23/26.
//

#include "fonts.h"

#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "fonts.impl.h"
#include "../debug.h"
#include "../memory.h"
#include "../hashmap.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../libs/stb_truetype.h"

typedef struct GLYPH_BITMAP_T {
    i32 width, height;
    u8s bytes;
} GlyphBitmap;

typedef struct HEIGHT_GLYPH_MAP_T {
    HASHMAP_FIELDS(i32, GlyphBitmap)
} HeightBitmapPairs;

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

    HeightBitmapPairs bitmaps; // per px height
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
            HASHMAP_FIELDS(u32, CodepointInfo)
        } non_ascii; // unicode key
    } codepoints;
};

static void codepoint_info_init(FontHandle *font, CodepointInfo *codepoint_info) {
    stbtt_GetGlyphHMetrics(
        &font->info,
        codepoint_info->glyph_idx,
        &codepoint_info->advance_width_units,
        &codepoint_info->left_side_bearing_units
    );

    stbtt_GetGlyphBox(
        &font->info,
        codepoint_info->glyph_idx,
        &codepoint_info->bbox.x0,
        &codepoint_info->bbox.y0,
        &codepoint_info->bbox.x1,
        &codepoint_info->bbox.y1
    );

    codepoint_info->bitmaps = (HeightBitmapPairs){
        .type = HASHMAP_TYPE_NON_STR_KEY,
        .del_freq = HASHMAP_DEL_FREQ_LOW,
    };
    HASHMAP_MAKE(&codepoint_info->bitmaps);
}

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
    HASHMAP_MAKE(&font->scale_factors);

    stbtt_GetFontBoundingBox(
        &font->info,
        &font->max_bbox.x0,
        &font->max_bbox.y0,
        &font->max_bbox.x1,
        &font->max_bbox.y1
    );

    for (i64 ascii_idx = 0; ascii_idx <= INT8_MAX; ascii_idx++) {
        CodepointInfo *ascii_info = &font->codepoints.ascii[ascii_idx];
        ascii_info->glyph_idx = stbtt_FindGlyphIndex(&font->info, (i32) ascii_idx);
        if (ascii_info->glyph_idx) {
            codepoint_info_init(font, ascii_info);
        }
    }

    font->codepoints.non_ascii.type = HASHMAP_TYPE_NON_STR_KEY;
    font->codepoints.non_ascii.del_freq = HASHMAP_DEL_FREQ_LOW;
    HASHMAP_MAKE(&font->codepoints.non_ascii);

    return font;
}

void font_unload(FontHandle *font) {
    arena_free(font->arena);
    alt_free(font);
}

static CodepointInfo *get_codepoint_info(FontHandle *font, const char *utf8) {
    CodepointInfo *info = nullptr;
    if (isascii(*utf8)) {
        info = &font->codepoints.ascii[*utf8];
    } else {
        u32 unicode = utf8_to_unicode(utf8, 4);
        auto unicode_info_pair = HASHMAP_GET(&font->codepoints.non_ascii, &unicode);
        if (!unicode_info_pair) {
            crash_msg("Codepoint info for %.*s not created yet\n", utf8_size(utf8, 4), utf8);
        }
        info = &unicode_info_pair->value;
    }

    return info;
}

f32x2 font_impl_measure_text_line(FontImplMeasureTextLineInfo *info) {
    f32 scale_factor = font_impl_get_scale_factor(info->font, info->height_px);

    f32 y1 = scale_factor * (f32) info->font->max_bbox.y1;
    f32 y0 = scale_factor * (f32) info->font->max_bbox.y0;
    i32 height = (i32) (y1 - y0);

    i32 width = 0;

    const char *prev_utf8 = nullptr;
    STRING_VIEW_FOR(utf8, &info->line) {
        i64 remaining_bytes = info->line.start + info->line.len - utf8;
        CodepointInfo *codepoint_info = get_codepoint_info(info->font, utf8);

        if (codepoint_info->glyph_idx > 0) {
            if (info->ui_measure_params.include_side_bearings.left && utf8 == info->line.start) {
                f32 lsb_px = scale_factor * (f32) codepoint_info->left_side_bearing_units;
                if (lsb_px < 0) {
                    width -= (i32) lsb_px;
                }
            }

            width += (i32) (scale_factor * (f32) font_impl_get_kerning_advance(info->font, prev_utf8, utf8));
            width += (i32) (scale_factor * (f32) codepoint_info->advance_width_units);

            if (utf8 + utf8_size(utf8, remaining_bytes) < info->line.start + info->line.len) {
                width += info->letter_spacing_px;
            } else if (info->ui_measure_params.include_side_bearings.right) {
                f32 rsb_px = scale_factor * (f32) (
                                 codepoint_info->advance_width_units - (
                                     codepoint_info->left_side_bearing_units + (
                                         codepoint_info->bbox.x1 - codepoint_info->bbox.x0)));
                if (rsb_px < 0) {
                    width -= (i32) rsb_px;
                }
            }
        }

        prev_utf8 = utf8;
    }

    f32x2 dim = {
        .width = (f32) width,
        .height = (f32) height,
    };

    return dim;
}

stbtt_fontinfo *font_impl_get_info(FontHandle *font) {
    return &font->info;
}

float font_impl_get_scale_factor(FontHandle *font, i32 px_height) {
    f32 sf = 0;
    auto px_sf_pair = HASHMAP_GET(&font->scale_factors, &px_height);
    if (!px_sf_pair) {
        sf = stbtt_ScaleForPixelHeight(&font->info, (f32) px_height);
        HASHMAP_PUT(&font->scale_factors, &px_height, &sf);
    } else {
        sf = px_sf_pair->value;
    }

    return sf;
}

i32 font_impl_get_glyph_idx(FontHandle *font, const char *codepoint) {
    i32 glyph_idx = 0;

    if (isascii(*codepoint)) {
        glyph_idx = font->codepoints.ascii[*codepoint].glyph_idx;
    } else {
        u32 unicode = utf8_to_unicode(codepoint, 4);
        auto unicode_info_pair = HASHMAP_GET(&font->codepoints.non_ascii, &unicode);
        if (!unicode_info_pair) {
            glyph_idx = stbtt_FindGlyphIndex(&font->info, (int) unicode);
            CodepointInfo unicode_info = {
                .glyph_idx = glyph_idx,
            };

            if (glyph_idx > 0) {
                codepoint_info_init(font, &unicode_info);
            }

            HASHMAP_PUT(&font->codepoints.non_ascii, &unicode, &unicode_info);
        } else {
            glyph_idx = unicode_info_pair->value.glyph_idx;
        }
    }

    return glyph_idx;
}


i32 font_impl_get_left_side_bearing(FontHandle *font, const char *codepoint) {
    i32 lsb = get_codepoint_info(font, codepoint)->left_side_bearing_units;
    return lsb;
}

i32 font_impl_get_advance_width(FontHandle *font, const char *codepoint) {
    i32 adv_width = get_codepoint_info(font, codepoint)->advance_width_units;
    return adv_width;
}

void font_impl_get_max_bbox(FontHandle *font, i32 *x0, i32 *y0, i32 *x1, i32 *y1) {
    *x0 = font->max_bbox.x0;
    *y0 = font->max_bbox.y0;
    *x1 = font->max_bbox.x1;
    *y1 = font->max_bbox.y1;
}

bool font_impl_codepoint_bitmap_exists(FontHandle *font, const char *codepoint, i32 px_height) {
    CodepointInfo *codepoint_info = get_codepoint_info(font, codepoint);
    bool exists = HASHMAP_GET(&codepoint_info->bitmaps, &px_height) != nullptr;
    return exists;
}

void font_impl_create_codepoint_bitmap(FontHandle *font, const char *codepoint, i32 px_height, i32 bitmap_width,
                                       i32 bitmap_height) {
    if (font_impl_codepoint_bitmap_exists(font, codepoint, px_height)) {
        return;
    }

    GlyphBitmap bitmap = {
        .bytes = {
            .arena = font->arena,
            .len = bitmap_width * bitmap_height,
        },
        .width = bitmap_width,
        .height = bitmap_height,
    };
    ARRAY_MAKE(&bitmap.bytes);

    HeightBitmapPairs *height_bitmap_map = &get_codepoint_info(font, codepoint)->bitmaps;

    HASHMAP_PUT(height_bitmap_map, &px_height, &bitmap);
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
        auto height_bitmap_pair = HASHMAP_GET(&font->codepoints.ascii[*codepoint].bitmaps, &px_height);
        if (!height_bitmap_pair) {
            crash_msg("Glyph bitmap for char %c and height %d px is missing\n", *codepoint, px_height);
        }

        glyph_bitmap = &height_bitmap_pair->value;
    } else {
        CodepointInfo *unicode_info = get_codepoint_info(font, codepoint);
        auto height_bitmap_pair = HASHMAP_GET(&unicode_info->bitmaps, &px_height);
        if (!height_bitmap_pair) {
            crash_msg("Glyph bitmap for unicode %.*s and height %d px is missing\n", utf8_size(codepoint, 4), codepoint,
                      px_height);
        }

        glyph_bitmap = &height_bitmap_pair->value;
    }

    *out_bitmap_bytes = glyph_bitmap->bytes.data;
    *out_bitmap_width = glyph_bitmap->width;
    *out_bitmap_height = glyph_bitmap->height;
}

i32 font_impl_get_kerning_advance(FontHandle *font, const char *prev_codepoint, const char *next_codepoint) {
    i32 advance = 0;

    if (!prev_codepoint || !next_codepoint) {
        return advance;
    }

    CodepointInfo *prev_info = get_codepoint_info(font, prev_codepoint);
    CodepointInfo *next_info = get_codepoint_info(font, next_codepoint);

    if (prev_info->glyph_idx <= 0 || next_info->glyph_idx <= 0) {
        return advance;
    }

    advance = stbtt_GetGlyphKernAdvance(&font->info, prev_info->glyph_idx, next_info->glyph_idx);

    return advance;
}
