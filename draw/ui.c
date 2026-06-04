//
// Created by wright on 5/23/26.
//

#include "ui.h"

#include <assert.h>

#include "fonts_impl.h"

static FontHandle **g_fonts = nullptr;
static i64 g_fonts_len = 0;

static RGBA8888 clay_to_render_color(Clay_Color clay_color) {
    return (RGBA8888){
        .r = (u8) clay_color.r,
        .g = (u8) clay_color.g,
        .b = (u8) clay_color.b,
        .a = (u8) clay_color.a,
    };
}

static RectCornerRadii clay_to_render_corner_radii(Clay_CornerRadius clay_radii) {
    return (RectCornerRadii){
        .top_left_px = f32_init(clay_radii.topLeft),
        .top_right_px = f32_init(clay_radii.topRight),
        .bottom_left_px = f32_init(clay_radii.bottomLeft),
        .bottom_right_px = f32_init(clay_radii.bottomRight)
    };
}

static f32x4 clay_to_render_rect(Clay_BoundingBox box) {
    return (f32x4){
        .start_x = f32_init(box.x),
        .start_y = f32_init(box.y),
        .width = f32_init(box.width),
        .height = f32_init(box.height),
    };
}

void ui_set_fonts(FontHandle **fonts, i64 fonts_len) {
    g_fonts = fonts;
    g_fonts_len = fonts_len;
}

Clay_Dimensions ui_clay_measure_text_fn(Clay_StringSlice text, Clay_TextElementConfig* config, void* user_data) {
    FontHandle* font = g_fonts[config->fontId];

    f32x2 dim = font_measure_text_line(
        font,
        (string_view){.start = text.chars, .len = text.length},
        config->fontSize,
        config->letterSpacing
    );

    return (Clay_Dimensions){
        .width = f32_float(dim.width),
        .height = f32_float(dim.height),
    };
};

Clay_Color ui_render_to_clay_color(RGBA8888 color) {
    return (Clay_Color){
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

void ui_clay_to_render_cmds(
    Framebuffer *canvas,
    RenderCmdBuffer *render_cmds,
    const Clay_RenderCommandArray *clay_cmds
) {
    for (i32 clay_cmd_idx = 0; clay_cmd_idx < clay_cmds->length; clay_cmd_idx++) {
        const Clay_RenderCommand *clay_cmd = &clay_cmds->internalArray[clay_cmd_idx];

        switch (clay_cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                RenderCmd rect_cmd = {RENDER_CMD_TYPE_DRAW_RECT};
                rect_cmd.data.draw_rect.framebuffer = canvas;

                rect_cmd.data.draw_rect.dst = clay_to_render_rect(clay_cmd->boundingBox);

                rect_cmd.data.draw_rect.bg_color = clay_to_render_color(
                    clay_cmd->renderData.rectangle.backgroundColor
                );

                rect_cmd.data.draw_rect.corner_radii = clay_to_render_corner_radii(
                    clay_cmd->renderData.rectangle.cornerRadius
                );

                ARRAY_PUSH(render_cmds, &rect_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                RenderCmd border_cmd = {RENDER_CMD_TYPE_DRAW_RECT};
                border_cmd.data.draw_rect.framebuffer = canvas;

                border_cmd.data.draw_rect.dst = clay_to_render_rect(clay_cmd->boundingBox);

                border_cmd.data.draw_rect.border_color = clay_to_render_color(
                    clay_cmd->renderData.border.color
                );

                border_cmd.data.draw_rect.border_widths = (RectBorderWidths){
                    .left_px = f32_init(clay_cmd->renderData.border.width.left),
                    .right_px = f32_init(clay_cmd->renderData.border.width.right),
                    .top_px = f32_init(clay_cmd->renderData.border.width.top),
                    .bottom_px = f32_init(clay_cmd->renderData.border.width.bottom),
                };

                border_cmd.data.draw_rect.corner_radii = clay_to_render_corner_radii(
                    clay_cmd->renderData.border.cornerRadius
                );

                ARRAY_PUSH(render_cmds, &border_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                RenderCmd text_cmd = {RENDER_CMD_TYPE_DRAW_TEXT};
                text_cmd.data.draw_text.framebuffer = canvas;
                text_cmd.data.draw_text.dst = clay_to_render_rect(
                    clay_cmd->boundingBox
                );

                assert(clay_cmd->renderData.text.fontId < g_fonts_len);
                text_cmd.data.draw_text.font = g_fonts[clay_cmd->renderData.text.fontId];

                text_cmd.data.draw_text.color = clay_to_render_color(
                    clay_cmd->renderData.text.textColor
                );

                text_cmd.data.draw_text.font_height_px = clay_cmd->renderData.text.fontSize;
                text_cmd.data.draw_text.letter_spacing_px = clay_cmd->renderData.text.letterSpacing;
                text_cmd.data.draw_text.line_height_px = clay_cmd->renderData.text.lineHeight;

                ARRAY_PUSH(render_cmds, &text_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                RenderCmd scissor_cmd = {RENDER_CMD_TYPE_SCISSOR};

                scissor_cmd.data.scissor.framebuffer = canvas;
                scissor_cmd.data.scissor.region = clay_to_render_rect(
                    clay_cmd->boundingBox
                );

                ARRAY_PUSH(render_cmds, &scissor_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                RenderCmd scissor_cmd = {RENDER_CMD_TYPE_SCISSOR};
                scissor_cmd.data.scissor.framebuffer = canvas;

                FramebufferInfo fb_info = framebuffer_get_info(canvas);
                assert(fb_info.type == FRAMEBUFFER_TYPE_PIXEL);

                scissor_cmd.data.scissor.region = (f32x4) {
                    .start_x = F32(0),
                    .start_y = F32(0),
                    .width = F32(fb_info.data.pixel_buf.size.width),
                    .height = F32(fb_info.data.pixel_buf.size.height),
                };

                ARRAY_PUSH(render_cmds, &scissor_cmd);

                break;
            }
            default:
                break;
        }
    }
}
