//
// Created by wright on 5/23/26.
//

#include "ui.h"

#include <assert.h>
#include <fcntl.h>

#include "fonts_impl.h"
#include "../../memory.h"

struct UI_CONTEXT_T {
    Clay_Arena arena;
    Clay_Context *ctx;
    Framebuffer *current_canvas;
};

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
        .top_left_px = clay_radii.topLeft,
        .top_right_px = clay_radii.topRight,
        .bottom_left_px = clay_radii.bottomLeft,
        .bottom_right_px = clay_radii.bottomRight
    };
}

static f32x4 clay_to_render_rect(Clay_BoundingBox box) {
    return (f32x4){
        .start_x = box.x,
        .start_y = box.y,
        .width = box.width,
        .height = box.height,
    };
}

Clay_Dimensions ui_clay_measure_text_fn(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) {
    FontHandle *font = g_fonts[config->fontId];

    f32x2 dim = font_measure_text_line(
        font,
        (string_view){.start = text.chars, .len = text.length},
        config->fontSize,
        config->letterSpacing
    );

    return (Clay_Dimensions){
        .width = dim.width,
        .height = dim.height,
    };
};

Clay_Color ui_clay_color(RGBA8888 color) {
    return (Clay_Color){
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

UiContext *ui_create(const UiCreateInfo *create_info) {
    UiContext *ui = alt_malloc(sizeof(*ui));
    *ui = (UiContext){};

    u64 memory_size = Clay_MinMemorySize();
    if (memory_size < create_info->memory_cap) {
        memory_size = create_info->memory_cap;
    }

    ui->arena = Clay_CreateArenaWithCapacityAndMemory(memory_size, alt_malloc(memory_size));
    FramebufferInfo canvas_info = framebuffer_get_info(create_info->initial_canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return nullptr;
    }
    i32x2 canvas_size = canvas_info.data.pixel_buf.size;
    ui->ctx = Clay_Initialize(
        ui->arena,
        (Clay_Dimensions){
            .width = (f32) canvas_size.width,
            .height = (f32) canvas_size.height
        },
        create_info->err_handler
    );

    return ui;
}

void ui_destroy(UiContext *ui) {
    alt_free(ui->arena.memory);
    alt_free(ui);
}

void ui_update_canvas(UiContext *ui, const Framebuffer *canvas) {
    FramebufferInfo fb_info = framebuffer_get_info(canvas);
}

void ui_set_fonts(FontHandle **fonts, i64 fonts_len) {
    g_fonts = fonts;
    g_fonts_len = fonts_len;
}


void ui_begin_layout(UiContext *ui, const UiBeginLayoutInfo *layout_info) {
    Clay_SetCurrentContext(ui->ctx);

    FramebufferInfo canvas_info = framebuffer_get_info(layout_info->canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return;
    }
    i32x2 canvas_size = canvas_info.data.pixel_buf.size;

    Clay_SetLayoutDimensions(
        (Clay_Dimensions){
            .width = (f32) canvas_size.width,
            .height = (f32) canvas_size.height
        }
    );

    ui->current_canvas = layout_info->canvas;

    Clay_SetPointerState(
        (Clay_Vector2){
            .x = layout_info->pointer_pos.x,
            .y = layout_info->pointer_pos.y
        },
        layout_info->pointer_pressed
    );

    Clay_UpdateScrollContainers(
        true,
        (Clay_Vector2){
            .x = layout_info->scroll_delta.x,
            .y = layout_info->scroll_delta.y,
        },
        layout_info->frame_elapsed_time_s
    );

    Clay_BeginLayout();
}

RenderCmds ui_end_layout(Arena *arena, UiContext *ui) {
    Clay_RenderCommandArray clay_cmds = Clay_EndLayout();

    RenderCmds render_cmds = {
        .arena = arena,
        .cap = clay_cmds.length,
    };
    ARRAY_MAKE(&render_cmds);

    for (i32 clay_cmd_idx = 0; clay_cmd_idx < clay_cmds.length; clay_cmd_idx++) {
        const Clay_RenderCommand *clay_cmd = &clay_cmds.internalArray[clay_cmd_idx];

        switch (clay_cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                RenderCmd rect_cmd = {RENDER_CMD_TYPE_DRAW_RECT};
                rect_cmd.data.draw_rect.framebuffer = ui->current_canvas;

                rect_cmd.data.draw_rect.dst = clay_to_render_rect(clay_cmd->boundingBox);

                rect_cmd.data.draw_rect.bg_color = clay_to_render_color(
                    clay_cmd->renderData.rectangle.backgroundColor
                );

                rect_cmd.data.draw_rect.corner_radii = clay_to_render_corner_radii(
                    clay_cmd->renderData.rectangle.cornerRadius
                );

                ARRAY_PUSH(&render_cmds, &rect_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                RenderCmd border_cmd = {RENDER_CMD_TYPE_DRAW_RECT};
                border_cmd.data.draw_rect.framebuffer = ui->current_canvas;

                border_cmd.data.draw_rect.dst = clay_to_render_rect(clay_cmd->boundingBox);

                border_cmd.data.draw_rect.border_color = clay_to_render_color(
                    clay_cmd->renderData.border.color
                );

                border_cmd.data.draw_rect.border_widths = (RectBorderWidths){
                    .left_px = clay_cmd->renderData.border.width.left,
                    .right_px = clay_cmd->renderData.border.width.right,
                    .top_px = clay_cmd->renderData.border.width.top,
                    .bottom_px = clay_cmd->renderData.border.width.bottom,
                };

                border_cmd.data.draw_rect.corner_radii = clay_to_render_corner_radii(
                    clay_cmd->renderData.border.cornerRadius
                );

                ARRAY_PUSH(&render_cmds, &border_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                RenderCmd text_cmd = {RENDER_CMD_TYPE_DRAW_TEXT};
                text_cmd.data.draw_text.framebuffer = ui->current_canvas;
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

                ARRAY_PUSH(&render_cmds, &text_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                RenderCmd scissor_cmd = {RENDER_CMD_TYPE_SCISSOR};

                scissor_cmd.data.scissor.framebuffer = ui->current_canvas;
                scissor_cmd.data.scissor.region = clay_to_render_rect(
                    clay_cmd->boundingBox
                );

                ARRAY_PUSH(&render_cmds, &scissor_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                RenderCmd scissor_cmd = {RENDER_CMD_TYPE_SCISSOR};
                scissor_cmd.data.scissor.framebuffer = ui->current_canvas;

                FramebufferInfo fb_info = framebuffer_get_info(ui->current_canvas);
                assert(fb_info.type == FRAMEBUFFER_TYPE_PIXEL);

                scissor_cmd.data.scissor.region = (f32x4){
                    .start_x = 0,
                    .start_y = 0,
                    .width = (f32) fb_info.data.pixel_buf.size.width,
                    .height = (f32) fb_info.data.pixel_buf.size.height,
                };

                ARRAY_PUSH(&render_cmds, &scissor_cmd);

                break;
            }
            default:
                break;
        }
    }

    return render_cmds;
}
