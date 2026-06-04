//
// Created by wright on 5/25/26.
//

#ifndef ALTCORE_DRAW_RECT_H
#define ALTCORE_DRAW_RECT_H

#include "../../types.h"
#include "../pixels.h"
#include "../renderer.h"
#include "../framebuffer.h"

void soft_cmd_draw_rect(
    Framebuffer* px_buf,
    i32x4 px_buf_dst,
    RGBA8888 bg_color,
    RectCornerRadii corner_radii,
    RGBA8888 border_color,
    RectBorderWidths border_widths
);

#endif //ALTCORE_DRAW_RECT_H
