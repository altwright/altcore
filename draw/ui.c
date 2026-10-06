//
// Created by wright on 5/23/26.
//

#include "ui.h"

#include <assert.h>

#include "fonts.impl.h"
#include "framebuffer.impl.h"
#include "ui.impl.h"
#include "../../memory.h"
#include "../../hashmap.h"
#include "../../strings.h"
#include "../../maths.h"
#include "../../debug.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

typedef struct LOCALE_KEY_MAP_T {
    HASHMAP_FIELDS(u64, string_view)
} LocaleKeyMap;

typedef struct STRING_KEY_MAP_T {
    HASHMAP_FIELDS(u64, LocaleKeyMap)
} StringKeyMap;

typedef struct FONT_THEMES_T {
    ARRAY_FIELDS(FontSets)
} FontThemes;

struct UI_CONTEXT_T {
    Arena *arena;

    Clay_Arena clay_arena;
    Clay_Context *clay_ctx;

    f32 viewport_aspect_ratio;

    struct {
        f32x2 offset;
        Framebuffer *fb;
    } current_canvas;

    u64 current_locale;
    StringKeyMap string_key_map;

    FontThemes font_themes;

    UiImplMeasureTextLineParams measure_text_line_params;
};

static void ui_error_handler(Clay_ErrorData err_data) {
    debug_msg(
        "UI ERROR %d: %.*s\n",
        err_data.errorType,
        err_data.errorText.length,
        err_data.errorText.chars
    );
}

static rgba8 clay_to_render_color(Clay_Color clay_color) {
    return (rgba8){
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

FontStyle ui_impl_read_font_modifier_flags(u16 font_mod_flags) {
    FontStyle font_style = FONT_STYLE_REGULAR;
    if ((font_mod_flags & UI_FONT_MODIFIER_ITALIC_FLAG) && (font_mod_flags & UI_FONT_MODIFIER_BOLD_FLAG)) {
        font_style = FONT_STYLE_BOLD_ITALIC;
    } else if (font_mod_flags & UI_FONT_MODIFIER_ITALIC_FLAG) {
        font_style = FONT_STYLE_ITALIC;
    } else if (font_mod_flags & UI_FONT_MODIFIER_BOLD_FLAG) {
        font_style = FONT_STYLE_BOLD;
    }

    return font_style;
}

Clay_Dimensions ui_impl_clay_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) {
    auto ui = (UiContext *) user_data;
    u16 font_theme_idx = config->fontId & 0xff;
    u16 font_mod_flags = config->fontId & ~font_theme_idx;

    FontSets *font_theme = ARRAY_GET(&ui->font_themes, font_theme_idx);

    FontStyle font_style = ui_impl_read_font_modifier_flags(font_mod_flags);

    string_view full_txt_line = {
        .start = text.chars,
        .len = text.length,
    };

    f32x2 final_dim = {};

    /*
     * We want to identify "runs" within the text that use the same font.
     * For example, if there was a line of mostly ascii characters with a
     * unicode character in the middle that is not found in the default FontHandle, the run of ascii
     * characters leading up to the unicode should be measured with the default FontHandle, the
     * unicode character should be measured with the fallback FontHandle that contains it, and the
     * remaining run of ascii characters should be measured with the default FontHandle again.
     * If, however, the string is all unicode characters, then the measure function should only
     * be called once with the fallback FontHandle that includes it.
     */
    const char *prev_txt_start = text.chars;
    i64 prev_font_set_idx = 0;
    STRING_VIEW_FOR(utf8, &full_txt_line) {
        i64 current_font_set_idx = 0;

        FontHandle *font = ARRAY_GET(font_theme, current_font_set_idx)->styles[font_style];

        while (font_impl_get_glyph_idx(font, utf8) <= 0) {
            current_font_set_idx++;
            if (current_font_set_idx >= font_theme->len) {
                current_font_set_idx = 0;
                break;
            }

            font = ARRAY_GET(font_theme, current_font_set_idx)->styles[font_style];
        }

        if (current_font_set_idx != prev_font_set_idx) {
            f32x2 dim = font_impl_measure_text_line(
                &(FontImplMeasureTextLineInfo){
                    .font = font,
                    .line = (string_view){
                        .start = prev_txt_start,
                        .len = utf8 - prev_txt_start
                    },
                    .height_px = config->fontSize,
                    .letter_spacing_px = config->letterSpacing,
                    .ui_measure_params = ui->measure_text_line_params
                }
            );

            final_dim.height = MAX(dim.height, final_dim.height);
            final_dim.width += dim.width;

            prev_txt_start = utf8;
            prev_font_set_idx = current_font_set_idx;
        }
    }

    f32x2 dim = font_impl_measure_text_line(
        &(FontImplMeasureTextLineInfo){
            .font = ARRAY_GET(font_theme, prev_font_set_idx)->styles[font_style],
            .line = (string_view){
                .start = prev_txt_start,
                .len = (full_txt_line.start + full_txt_line.len) - prev_txt_start
            },
            .height_px = config->fontSize,
            .letter_spacing_px = config->letterSpacing,
            .ui_measure_params = ui->measure_text_line_params
        }
    );

    final_dim.height = MAX(dim.height, final_dim.height);
    final_dim.width += dim.width;

    return (Clay_Dimensions){
        .width = final_dim.width,
        .height = final_dim.height,
    };
};

Clay_Color ui_color(rgba8 color) {
    return (Clay_Color){
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

UiContext *ui_create(const UiCreateInfo *create_info) {
    UiContext *ui = alt_malloc(sizeof(*ui));
    *ui = (UiContext){
        .viewport_aspect_ratio = MAX(0, create_info->viewport_aspect_ratio),
    };

    u64 memory_size = Clay_MinMemorySize();
    if (memory_size < create_info->memory_cap) {
        memory_size = create_info->memory_cap;
    }

    ui->arena = arena_make((i64) memory_size + (i64) MIBIBYTE);

    ui->clay_arena = Clay_CreateArenaWithCapacityAndMemory(memory_size, arena_alloc(ui->arena, (i64) memory_size));
    FramebufferInfo canvas_info = framebuffer_get_info(create_info->initial_canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Framebuffer is not a pixel buffer\n");
    }

    f32x2 canvas_size = itof32x2(canvas_info.data.pixel_buf.size);
    Clay_Dimensions ui_size = {
        .width = canvas_size.width,
        .height = canvas_size.height
    };

    if (ui->viewport_aspect_ratio) {
        f32 canvas_aspect_ratio = canvas_size.width / canvas_size.height;
        if (canvas_aspect_ratio > ui->viewport_aspect_ratio) {
            ui_size.width = ui_size.height * ui->viewport_aspect_ratio;
        } else {
            ui_size.height = ui_size.width / ui->viewport_aspect_ratio;
        }
    }

    ui->clay_ctx = Clay_Initialize(
        ui->clay_arena,
        ui_size,
        (Clay_ErrorHandler){
            .errorHandlerFunction = ui_error_handler
        }
    );

    ui->font_themes = (FontThemes){
        .arena = ui->arena,
        .len = create_info->font_themes.len,
    };
    ARRAY_MAKE(&ui->font_themes);

    for (i64 font_theme_idx = 0; font_theme_idx < create_info->font_themes.len; font_theme_idx++) {
        UiFontSets *create_font_theme = &create_info->font_themes.data[font_theme_idx];
        FontSets *font_theme = ARRAY_GET(&ui->font_themes, font_theme_idx);
        *font_theme = (FontSets){
            .arena = ui->arena,
            .len = create_font_theme->len,
        };
        ARRAY_MAKE(font_theme);

        for (i64 font_set_idx = 0; font_set_idx < create_font_theme->len; font_set_idx++) {
            FontSet *create_font_set = &create_font_theme->data[font_set_idx];
            FontSet *font_set = ARRAY_GET(font_theme, font_set_idx);
            memcpy(font_set, create_font_set, sizeof(FontSet));
        }
    }

    ui->string_key_map = (StringKeyMap){
        .type = HASHMAP_TYPE_NON_STR_KEY,
        .del_freq = HASHMAP_DEL_FREQ_LOW,
    };
    HASHMAP_MAKE(&ui->string_key_map);

    // Default parameters for drawing a single line of text
    ui->measure_text_line_params = (UiImplMeasureTextLineParams){
        .include_side_bearings = {
            .left = true,
            .right = true,
        },
    };

    return ui;
}

void ui_destroy(UiContext *ui) {
    arena_free(ui->arena);
    HASHMAP_FREE(&ui->string_key_map);
    alt_free(ui);
}

void ui_begin_layout(UiContext *ui, const UiBeginLayoutInfo *layout_info) {
    Clay_SetCurrentContext(ui->clay_ctx);

    FramebufferInfo canvas_info = framebuffer_get_info(layout_info->canvas);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Expected a pixel framebuffer\n");
    }
    f32x2 canvas_size = itof32x2(canvas_info.data.pixel_buf.size);

    Clay_Dimensions ui_size = {
        .width = canvas_size.width,
        .height = canvas_size.height
    };

    if (ui->viewport_aspect_ratio) {
        f32 canvas_aspect_ratio = canvas_size.width / canvas_size.height;
        if (canvas_aspect_ratio > ui->viewport_aspect_ratio) {
            ui_size.width = ui_size.height * ui->viewport_aspect_ratio;
            ui->current_canvas.offset.x = (canvas_size.width - ui_size.width) / 2;
        } else {
            ui_size.height = ui_size.width / ui->viewport_aspect_ratio;
            ui->current_canvas.offset.y = (canvas_size.height - ui_size.height) / 2;
        }
    }

    Clay_SetLayoutDimensions(ui_size);

    ui->current_canvas.fb = layout_info->canvas;

    Clay_SetPointerState(
        (Clay_Vector2){
            .x = layout_info->mouse.pointer.pos.curr_frame.x - ui->current_canvas.offset.x,
            .y = layout_info->mouse.pointer.pos.curr_frame.y - ui->current_canvas.offset.y,
        },
        layout_info->mouse.pointer.pressed.curr_frame != 0
    );

    Clay_UpdateScrollContainers(
        true,
        (Clay_Vector2){
            .x = layout_info->mouse.scroll_delta.x,
            .y = layout_info->mouse.scroll_delta.y,
        },
        layout_info->frame_elapsed_time_s
    );

    Clay_SetMeasureTextFunction(ui_impl_clay_measure_text, ui);

    Clay_BeginLayout();
}

RenderCmds ui_end_layout(Arena *arena, UiContext *ui) {
    Clay_RenderCommandArray clay_cmds = Clay_EndLayout();

    RenderCmds render_cmds = {
        .arena = arena,
        .cap = clay_cmds.length,
    };
    ARRAY_MAKE(&render_cmds);

    f32x4 canvas_region_offset = {
        .start_x = ui->current_canvas.offset.x,
        .start_y = ui->current_canvas.offset.y,
    };

    for (i32 clay_cmd_idx = 0; clay_cmd_idx < clay_cmds.length; clay_cmd_idx++) {
        const Clay_RenderCommand *clay_cmd = &clay_cmds.internalArray[clay_cmd_idx];

        f32x4 dst_region = f32x4_add(clay_to_render_rect(clay_cmd->boundingBox), canvas_region_offset);

        switch (clay_cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                RenderCmd rect_cmd = {RENDER_CMD_TYPE_DRAW_RECT};
                rect_cmd.data.draw_rect.dst_framebuffer = ui->current_canvas.fb;

                rect_cmd.data.draw_rect.dst_region = dst_region;

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
                border_cmd.data.draw_rect.dst_framebuffer = ui->current_canvas.fb;

                border_cmd.data.draw_rect.dst_region = dst_region;

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

                render_cmd_data->dst_fb = ui->current_canvas.fb;
                render_cmd_data->dst_fb_region = dst_region;

                u16 font_theme_idx = clay_cmd->renderData.text.fontId & 0xff;
                u16 font_style_flags = clay_cmd->renderData.text.fontId & ~font_theme_idx;

                render_cmd_data->font_sets = ARRAY_GET(&ui->font_themes, font_theme_idx);
                render_cmd_data->font_style = ui_impl_read_font_modifier_flags(font_style_flags);

                render_cmd_data->text_color = clay_to_render_color(
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

                scissor_cmd.data.scissor.framebuffer = ui->current_canvas.fb;
                scissor_cmd.data.scissor.region = dst_region;

                ARRAY_PUSH(&render_cmds, &scissor_cmd);

                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                RenderCmd scissor_cmd = {RENDER_CMD_TYPE_SCISSOR};
                scissor_cmd.data.scissor.framebuffer = ui->current_canvas.fb;

                FramebufferInfo fb_info = framebuffer_get_info(ui->current_canvas.fb);
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
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                const Clay_ImageRenderData *image_cmd = &clay_cmd->renderData.image;

                RenderCmd blit_cmd = {
                    .type = RENDER_CMD_TYPE_DRAW_RECT,
                };
                RenderCmdDrawRect *blit_data = &blit_cmd.data.draw_rect;

                blit_data->dst_framebuffer = ui->current_canvas.fb;
                blit_data->dst_region = dst_region;
                blit_data->corner_radii = clay_to_render_corner_radii(image_cmd->cornerRadius);

                Framebuffer *image_fb = image_cmd->imageData;
                FramebufferInfo image_fb_info = framebuffer_get_info(image_fb);

                if (image_fb_info.type != FRAMEBUFFER_TYPE_PIXEL) {
                    crash_msg("Expected pixel framebuffer, not type %d\n", image_fb_info.type);
                }

                u8 *image_fb_bytes = framebuffer_impl_get_bytes(image_fb);
                blit_data->src_blit.pixel_bytes = image_fb_bytes;
                blit_data->src_blit.px_format = image_fb_info.data.pixel_buf.format;
                blit_data->src_blit.size = image_fb_info.data.pixel_buf.size;
                blit_data->src_blit.pitch_bytes = image_fb_info.data.pixel_buf.pitch_bytes;

                ARRAY_PUSH(&render_cmds, &blit_cmd);

                break;
            }
            default:
                crash_msg("Unhandled Clay render command %d\n", clay_cmd->commandType);
                break;
        }
    }

    Clay_SetMeasureTextFunction(nullptr, nullptr);

    ui->current_canvas.offset = (f32x2){.x = 0, .y = 0};
    ui->current_canvas.fb = nullptr;

    return render_cmds;
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
        .isStaticallyAllocated = true
    };
}

void ui_set_locale(UiContext *ui, u64 loc_key) {
    ui->current_locale = loc_key;
}

u64 ui_get_locale(UiContext *ui) {
    return ui->current_locale;
}

u16 ui_height_px(UiContext *ui, f64 pct) {
    if (!ui->current_canvas.fb) {
        return 0;
    }

    FramebufferInfo canvas_info = framebuffer_get_info(ui->current_canvas.fb);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return 0;
    }

    i32x2 canvas_size = canvas_info.data.pixel_buf.size;

    i32 px_height = (i32) ((f32) canvas_size.height * pct);

    return MAX(1, px_height);
}

u16 ui_width_px(UiContext *ui, f64 pct) {
    if (!ui->current_canvas.fb) {
        return 0;
    }

    FramebufferInfo canvas_info = framebuffer_get_info(ui->current_canvas.fb);
    if (canvas_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        return 0;
    }

    i32x2 canvas_size = canvas_info.data.pixel_buf.size;

    i32 px_width = (i32) ((f32) canvas_size.width * pct);

    return MAX(1, px_width);
}

f32x2 ui_viewport_coord(UiContext *ui, f32x2 canvas_coord) {
    return (f32x2){
        .x = canvas_coord.x - ui->current_canvas.offset.x,
        .y = canvas_coord.y - ui->current_canvas.offset.y,
    };
}

FontSets *ui_impl_get_font_sets(UiContext *ui, i32 font_theme_idx) {
    return ARRAY_GET(&ui->font_themes, font_theme_idx);
}

static bool pointer_pressed(UiMouseInfo *info, bool this_frame, UiMousePointerActionFlag action) {
    bool clicked = Clay_Hovered() && (info->pointer.pressed.curr_frame & action);
    if (this_frame) {
        clicked = clicked && !(info->pointer.pressed.prev_frame & action);
    }

    return clicked;
}

static bool pointer_released(UiMouseInfo *info, bool this_frame, UiMousePointerActionFlag action) {
    bool released = Clay_Hovered() && !(info->pointer.pressed.curr_frame & action);
    if (this_frame) {
        released = released && (info->pointer.pressed.prev_frame & action);
    }

    return released;
}

bool ui_elem_double_click(UiMouseInfo *info, bool is_pressed, bool this_frame) {
    return is_pressed
               ? pointer_pressed(info, this_frame, UI_MOUSE_POINTER_ACTION_DOUBLE_CLICK_FLAG)
               : pointer_released(info, this_frame, UI_MOUSE_POINTER_ACTION_DOUBLE_CLICK_FLAG);
}

bool ui_elem_left_button(UiMouseInfo *info, bool is_pressed, bool this_frame) {
    return is_pressed
               ? pointer_pressed(info, this_frame, UI_MOUSE_POINTER_ACTION_LEFT_CLICK_FLAG)
               : pointer_released(info, this_frame, UI_MOUSE_POINTER_ACTION_LEFT_CLICK_FLAG);
}

bool ui_elem_right_button(UiMouseInfo *info, bool is_pressed, bool this_frame) {
    return is_pressed
               ? pointer_pressed(info, this_frame, UI_MOUSE_POINTER_ACTION_RIGHT_CLICK_FLAG)
               : pointer_released(info, this_frame, UI_MOUSE_POINTER_ACTION_RIGHT_CLICK_FLAG);
}

bool ui_elem_middle_button(UiMouseInfo *info, bool is_pressed, bool this_frame) {
    return is_pressed
               ? pointer_pressed(info, this_frame, UI_MOUSE_POINTER_ACTION_MIDDLE_CLICK_FLAG)
               : pointer_released(info, this_frame, UI_MOUSE_POINTER_ACTION_MIDDLE_CLICK_FLAG);
}

UiImplMeasureTextLineParams ui_impl_get_measure_text_line_params(UiContext *ui) {
    return ui->measure_text_line_params;
}

void ui_impl_set_measure_text_line_params(UiContext *ui, const UiImplMeasureTextLineParams *params) {
    ui->measure_text_line_params = *params;
}
