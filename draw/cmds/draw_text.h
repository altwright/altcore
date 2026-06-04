//
// Created by wright on 5/25/26.
//

#ifndef ALTCORE_DRAW_TEXT_H
#define ALTCORE_DRAW_TEXT_H

#include "../../types.h"
#include "../../strings.h"
#include "../pixels.h"
#include "../framebuffer.h"
#include "../fonts.h"

void soft_cmd_draw_text(
    Framebuffer *px_buf,
    i32x4 px_buf_dst,
    string_view text,
    FontHandle *font,
    i32 font_size_px,
    i32 letter_spacing_px,
    RGBA8888 color
);

#endif //ALTCORE_DRAW_TEXT_H
