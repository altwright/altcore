//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_FONTS_H
#define ALTCORE_FONTS_H

#include "../types.h"
#include "../strings.h"

typedef struct FONT_HANDLE_T FontHandle;

typedef struct FONT_LOAD_INFO_T {
    struct {
        u8 *data;
        i64 len;
    } ttf;
} FontLoadInfo;

typedef enum FONT_STYLE_E {
#define X_FONT_STYLES \
    X(REGULAR) \
    X(ITALIC) \
    X(BOLD) \
    X(BOLD_ITALIC) \
    X(COUNT)
#define X(style) \
    FONT_STYLE_##style,
    X_FONT_STYLES
#undef X
} FontStyle;

typedef struct FONT_SET_T {
    FontHandle *styles[FONT_STYLE_COUNT];
} FontSet;

typedef struct FONT_SETS_T {
    ARRAY_FIELDS(FontSet)
} FontSets;

FontHandle *font_load(const FontLoadInfo *info);

void font_unload(FontHandle *font);

f32x2 font_measure_text(
    FontHandle *font,
    string_view line,
    i32 height_px,
    i32 letter_spacing_px
);

#endif //ALTCORE_FONTS_H
