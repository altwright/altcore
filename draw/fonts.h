//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_FONTS_H
#define ALTCORE_FONTS_H

#include "../types.h"
#include "../strings.h"
#include "../fs.h"

struct FONT_HANDLE_T;
typedef struct FONT_HANDLE_T FontHandle;

FontHandle *font_load_ttf(Filepath* path);

void font_unload(FontHandle *font);

f32x2 font_measure_text_line(
    FontHandle* font,
    string_view view,
    i32 height_px,
    i32 letter_spacing_px
);

#endif //ALTCORE_FONTS_H
