//
// Created by wright on 5/23/26.
//

#include "ui.h"

#include <assert.h>

#include "fonts.impl.h"
#include "../../memory.h"
#include "../../hashmap.h"
#include "../../strings.h"
#include "../../maths.h"
#include "../../debug.h"

typedef struct LOCALE_KEY_MAP_T {
    HASHMAP_FIELDS(u64, string_view)
} LocaleKeyMap;

typedef struct STRING_KEY_MAP_T {
    HASHMAP_FIELDS(u64, LocaleKeyMap)
} StringKeyMap;

struct UI_CONTEXT_T {
    Clay_Arena clay_arena;
    Clay_Context *clay_ctx;
    Framebuffer *current_canvas;

    struct {
        FontHandle **data;
        i64 len;
    } fonts;

    u64 current_locale;
    StringKeyMap string_key_map;
};

static void ui_error_handler(Clay_ErrorData err_data) {
    debug_msg(
        "UI ERROR %d: %.*s\n",
        err_data.errorType,
        err_data.errorText.length,
        err_data.errorText.chars
    );
}

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

static Clay_Dimensions ui_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) {
    UiContext *ui = user_data;
    if (config->fontId >= ui->fonts.len) {
        crash_msg("Font index %d exceeds font array of length %d\n", config->fontId, ui->fonts.len);
    }

    FontHandle *font = ui->fonts.data[config->fontId];

    f32x2 dim = font_measure_text(
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

Clay_Color ui_color(RGBA8888 color) {
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

    ui->clay_arena = Clay_CreateArenaWithCapacityAndMemory(memory_size, alt_malloc(memory_size));
    FramebufferInfo canvas_info = framebuffer_get_info(create_info->initial_canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return nullptr;
    }
    i32x2 canvas_size = canvas_info.data.pixel_buf.size;
    ui->clay_ctx = Clay_Initialize(
        ui->clay_arena,
        (Clay_Dimensions){
            .width = (f32) canvas_size.width,
            .height = (f32) canvas_size.height
        },
        (Clay_ErrorHandler){
            .errorHandlerFunction = ui_error_handler
        }
    );

    u64 fonts_size = create_info->fonts.len * sizeof(*create_info->fonts.data);
    ui->fonts.data = alt_malloc(fonts_size);
    ui->fonts.len = create_info->fonts.len;
    memcpy(ui->fonts.data, create_info->fonts.data, fonts_size);

    ui->string_key_map = (StringKeyMap){
        .type = HASHMAP_TYPE_NON_STR_KEY,
        .del_freq = HASHMAP_DEL_FREQ_LOW,
    };
    HASHMAP_MAKE(&ui->string_key_map);

    return ui;
}

void ui_destroy(UiContext *ui) {
    alt_free(ui->fonts.data);
    HASHMAP_FREE(&ui->string_key_map);
    alt_free(ui->clay_arena.memory);
    alt_free(ui);
}

void ui_begin_layout(UiContext *ui, const UiBeginLayoutInfo *layout_info) {
    Clay_SetCurrentContext(ui->clay_ctx);

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

    Clay_SetMeasureTextFunction(ui_measure_text, ui);

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
                RenderCmd text_cmd = {.type = RENDER_CMD_TYPE_DRAW_TEXT};
                const Clay_TextRenderData *clay_cmd_data = &clay_cmd->renderData.text;
                RenderCmdDrawText *render_cmd_data = &text_cmd.data.draw_text;

                render_cmd_data->text.start = clay_cmd_data->stringContents.chars;
                render_cmd_data->text.len = clay_cmd_data->stringContents.length;

                render_cmd_data->framebuffer = ui->current_canvas;
                render_cmd_data->dst = clay_to_render_rect(
                    clay_cmd->boundingBox
                );

                i64 font_idx = clay_cmd->renderData.text.fontId;
                if (font_idx >= ui->fonts.len) {
                    crash_msg("Font index %d exceeds font array of length %d\n", font_idx, ui->fonts.len);
                }

                render_cmd_data->font = ui->fonts.data[clay_cmd->renderData.text.fontId];

                render_cmd_data->color = clay_to_render_color(
                    clay_cmd->renderData.text.textColor
                );

                render_cmd_data->font_height_px = clay_cmd->renderData.text.fontSize;
                render_cmd_data->letter_spacing_px = clay_cmd->renderData.text.letterSpacing;
                render_cmd_data->line_height_px = clay_cmd->renderData.text.lineHeight;

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

    Clay_SetMeasureTextFunction(nullptr, nullptr);

    ui->current_canvas = nullptr;

    return render_cmds;
}

Clay_Padding ui_padding(UiContext *ui, f32x4 padding_pct) {
    Clay_Padding clay_padding = {};

    if (!ui->current_canvas) {
        return clay_padding;
    }

    FramebufferInfo canvas_info = framebuffer_get_info(ui->current_canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return clay_padding;
    }

    i32x2 canvas_size = canvas_info.data.pixel_buf.size;
    clay_padding.left = (u16) ((f32) canvas_size.width * padding_pct.left);
    clay_padding.right = (u16) ((f32) canvas_size.width * padding_pct.right);
    clay_padding.top = (u16) ((f32) canvas_size.height * padding_pct.top);
    clay_padding.bottom = (u16) ((f32) canvas_size.height * padding_pct.bottom);

    return clay_padding;
}

Clay_Padding ui_padding_all(UiContext *ui, f32 padding_pct) {
    return ui_padding(
        ui,
        (f32x4){
            .left = padding_pct,
            .right = padding_pct,
            .top = padding_pct,
            .bottom = padding_pct
        }
    );
}

void ui_set_string(UiContext *ui, u64 str_key, u64 loc_key, const char8_t *utf8_str) {
    const char *c_str = (const char *) utf8_str;

    auto str_loc_pair = HASHMAP_GET(&ui->string_key_map, &str_key);
    if (!str_loc_pair) {
        LocaleKeyMap new_loc_map = {
            .type = HASHMAP_TYPE_NON_STR_KEY,
            .del_freq = HASHMAP_DEL_FREQ_LOW,
        };
        HASHMAP_MAKE(&new_loc_map);

        HASHMAP_PUT(&ui->string_key_map, &str_key, &new_loc_map);

        str_loc_pair = HASHMAP_GET(&ui->string_key_map, &str_key);
    }

    LocaleKeyMap *loc_map = &str_loc_pair->value;

    string_view view = {
        .start = c_str,
        .len = (i64) strlen(c_str),
    };
    HASHMAP_PUT(loc_map, &loc_key, &view);
}

Clay_String ui_get_string(UiContext *ui, u64 str_key) {
    auto str_loc_pair = HASHMAP_GET(&ui->string_key_map, &str_key);
    if (!str_loc_pair) {
        return (Clay_String){};
    }

    LocaleKeyMap *loc_map = &str_loc_pair->value;
    auto loc_str_pair = HASHMAP_GET(loc_map, &ui->current_locale);
    if (!loc_str_pair) {
        u64 default_loc_key = 0;
        loc_str_pair = HASHMAP_GET(loc_map, &default_loc_key);
    }

    string_view view = loc_str_pair->value;

    return (Clay_String){
        .chars = view.start,
        .length = (i32) view.len,
        .isStaticallyAllocated = true,
    };
}

void ui_set_locale(UiContext *ui, u64 loc_key) {
    ui->current_locale = loc_key;
}

u64 ui_get_locale(UiContext *ui) {
    return ui->current_locale;
}

void ui_set_fonts(UiContext *ui, FontHandle **fonts, i64 fonts_len) {
}

u16 ui_px_height(UiContext *ui, f32 pct) {
    if (!ui->current_canvas) {
        return 0;
    }

    FramebufferInfo canvas_info = framebuffer_get_info(ui->current_canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return 0;
    }

    i32x2 canvas_size = canvas_info.data.pixel_buf.size;

    i32 px_height = (i32) ((f32) canvas_size.height * pct);

    return MAX(1, px_height);
}
