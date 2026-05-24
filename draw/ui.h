//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_UI_H
#define ALTCORE_UI_H

#include "clay.h"
#include "renderer.h"

void ui_set_fonts(FontHandle **fonts, i64 fonts_len);

Clay_Color ui_render_to_clay_color(RGBA8888 color);

void ui_clay_to_render_cmds(
    Framebuffer *canvas,
    RenderCmdBuffer *render_cmds,
    const Clay_RenderCommandArray *clay_cmds
);

#endif //ALTCORE_UI_H
