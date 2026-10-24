//
// Created by wright on 5/25/26.
//

#include "draw_rect.h"

#include <assert.h>
#include <string.h>

#include "../../maths.h"
#include "../framebuffer.impl.h"
#include "../../debug.h"

static bool px_beyond_corner_axes(f32x2 px_pos,
                                  f32x2 top_left_axis,
                                  f32x2 top_right_axis,
                                  f32x2 bottom_left_axis,
                                  f32x2 bottom_right_axis) {
    bool beyond = false;
    if ((px_pos.y <= top_left_axis.y && px_pos.x <= top_left_axis.x)
        || (px_pos.y <= top_right_axis.y && px_pos.x >= top_right_axis.x)
        || (px_pos.y >= bottom_left_axis.y && px_pos.x <= bottom_left_axis.x)
        || (px_pos.y >= bottom_right_axis.y && px_pos.x >= bottom_right_axis.x)) {
        beyond = true;
    }

    return beyond;
}

static bool px_outside_rounded_corner(f32x2 px_pos,
                                      f32x2 top_left_axis,
                                      f32x2 top_right_axis,
                                      f32x2 bottom_left_axis,
                                      f32x2 bottom_right_axis,
                                      RenderCmdDrawRectCornerRadii corner_radii
) {
    bool outside = true;
    if (px_pos.y <= top_left_axis.y && px_pos.x <= top_left_axis.x) {
        f32 dist = f32x2_dist(
            top_left_axis,
            px_pos
        );

        if (dist <= corner_radii.top_left_px) {
            outside = false;
        }
    } else if (px_pos.y <= top_right_axis.y && px_pos.x >= top_right_axis.x) {
        f32 dist = f32x2_dist(
            top_right_axis,
            px_pos
        );

        if (dist <= corner_radii.top_right_px) {
            outside = false;
        }
    } else if (px_pos.y >= bottom_left_axis.y && px_pos.x <= bottom_left_axis.x) {
        f32 dist = f32x2_dist(
            bottom_left_axis,
            px_pos
        );

        if (dist <= corner_radii.bottom_left_px) {
            outside = false;
        }
    } else if (px_pos.y >= bottom_right_axis.y && px_pos.x >= bottom_right_axis.x) {
        f32 dist = f32x2_dist(
            bottom_right_axis,
            px_pos
        );

        if (dist <= corner_radii.bottom_right_px) {
            outside = false;
        }
    }

    return outside;
}

static bool px_within_rounded_border(f32x2 px_pos,
                                     f32x2 top_left_axis,
                                     f32x2 top_right_axis,
                                     f32x2 bottom_left_axis,
                                     f32x2 bottom_right_axis,
                                     RenderCmdDrawRectCornerRadii corner_radii,
                                     RenderCmdDrawRectBorderWidths border_widths) {
    bool within = false;
    if (px_pos.y <= top_left_axis.y && px_pos.x <= top_left_axis.x) {
        f32 dist = f32x2_dist(
            top_left_axis,
            px_pos
        );

        if (dist <= corner_radii.top_left_px) {
            f32x2 vec = f32x2_sub(px_pos, top_left_axis);
            if (vec.y < 0) {
                vec.y *= -1;
            }
            if (vec.x != 0 || vec.y != 0) {
                f32 x_axis_rad = atan2f(vec.y, vec.x);
                if (x_axis_rad < 0) {
                    x_axis_rad *= -1;
                }
                f32 lerp = (x_axis_rad - (f32) M_PI_2) / (f32) M_PI_2;
                f32 lerp_border_width = f32_lerp(border_widths.top_px, border_widths.left_px, lerp);
                if (corner_radii.top_left_px - dist <= lerp_border_width) {
                    within = true;
                }
            }
        }
    } else if (px_pos.y <= top_right_axis.y && px_pos.x >= top_right_axis.x) {
        f32 dist = f32x2_dist(
            top_right_axis,
            px_pos
        );

        if (dist <= corner_radii.top_right_px) {
            f32x2 vec = f32x2_sub(px_pos, top_right_axis);
            if (vec.y < 0) {
                vec.y *= -1;
            }

            if (vec.x != 0 || vec.y != 0) {
                f32 x_axis_rad = atan2f(vec.y, vec.x);
                if (x_axis_rad < 0) {
                    x_axis_rad *= -1;
                }
                f32 lerp = x_axis_rad / (f32) M_PI_2;
                f32 lerp_border_width = f32_lerp(border_widths.right_px, border_widths.top_px, lerp);
                if (corner_radii.top_right_px - dist <= lerp_border_width) {
                    within = true;
                }
            }
        }
    } else if (px_pos.y >= bottom_left_axis.y && px_pos.x <= bottom_left_axis.x) {
        f32 dist = f32x2_dist(
            bottom_left_axis,
            px_pos
        );

        if (dist <= corner_radii.bottom_left_px) {
            f32x2 vec = f32x2_sub(px_pos, bottom_left_axis);
            if (vec.y > 0) {
                vec.y *= -1;
            }
            if (vec.x != 0 || vec.y != 0) {
                f32 x_axis_rad = atan2f(vec.y, vec.x);
                if (x_axis_rad > 0) {
                    x_axis_rad *= -1;
                }
                f32 lerp = (x_axis_rad + (f32) M_PI) / (f32) M_PI_2;
                f32 lerp_border_width = f32_lerp(border_widths.left_px, border_widths.bottom_px, lerp);
                if (corner_radii.bottom_left_px - dist <= lerp_border_width) {
                    within = true;
                }
            }
        }
    } else if (px_pos.y >= bottom_right_axis.y && px_pos.x >= bottom_right_axis.x) {
        f32 dist = f32x2_dist(
            bottom_right_axis,
            px_pos
        );

        if (dist <= corner_radii.bottom_right_px) {
            f32x2 vec = f32x2_sub(px_pos, bottom_right_axis);

            if (vec.y > 0) {
                vec.y *= -1;
            }

            if (vec.x != 0 || vec.y != 0) {
                f32 x_axis_rad = atan2f(vec.y, vec.x);
                if (x_axis_rad > 0) {
                    x_axis_rad *= -1;
                }

                f32 lerp = x_axis_rad / (f32) -M_PI_2;
                f32 lerp_border_width = f32_lerp(border_widths.right_px, border_widths.bottom_px, lerp);
                if (corner_radii.bottom_left_px - dist <= lerp_border_width) {
                    within = true;
                }
            }
        }
    }

    return within;
}

static bool px_within_straight_border(f32x2 px_pos, f32x4 region, RenderCmdDrawRectBorderWidths widths) {
    return px_pos.y - region.start_y < widths.top_px
           || (region.start_y + region.height) - px_pos.y <= widths.bottom_px
           || px_pos.x - region.start_x < widths.left_px
           || (region.start_x + region.width) - px_pos.x <= widths.right_px;
}

void soft_cmd_draw_rect(RenderCmdDrawRect *cmd) {
    FramebufferInfo dst_fb_info = framebuffer_get_info(cmd->dst);
    if (dst_fb_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Destination framebuffer is not a pixel buffer\n");
    }

    u8 *dst_fb_bytes = nullptr;
    i64 dst_fb_pitch_bytes = 0;
    framebuffer_impl_get_bytes(cmd->dst, &dst_fb_bytes, &dst_fb_pitch_bytes);

    PixelFormat dst_px_format = dst_fb_info.data.pixel.format;
    i64 dst_px_size = pixel_size(dst_px_format);
    i32x2 dst_fb_size = dst_fb_info.data.pixel.size;
    i32x4 dst_fb_region = ftoi32x4(cmd->dst_region);
    i32x4 dst_fb_scissor = ftoi32x4(dst_fb_info.data.pixel.scissor);
    RenderCmdDrawRectCornerRadii corner_radii = cmd->corner_radii;
    bool not_rounded = !corner_radii.top_left_px
                       && !corner_radii.top_right_px
                       && !corner_radii.bottom_left_px
                       && !corner_radii.bottom_right_px;

    i32 dst_start_y = CLAMP(dst_fb_region.start_y, 0, dst_fb_size.height);
    i32 dst_end_y = CLAMP(dst_fb_region.start_y + dst_fb_region.height, dst_start_y, dst_fb_size.height);
    i32 dst_start_x = CLAMP(dst_fb_region.start_x, 0, dst_fb_size.width);
    i32 dst_end_x = CLAMP(dst_fb_region.start_x + dst_fb_region.width, dst_start_x, dst_fb_size.width);

    dst_start_y = CLAMP(dst_start_y, dst_fb_scissor.start_y, dst_fb_size.height);
    dst_end_y = CLAMP(dst_end_y, dst_start_y, dst_fb_scissor.start_y + dst_fb_scissor.height);
    dst_start_x = CLAMP(dst_start_x, dst_fb_scissor.start_x, dst_fb_size.width);
    dst_end_x = CLAMP(dst_end_x, dst_start_x, dst_fb_scissor.start_x + dst_fb_scissor.width);

    f32x2 top_left_axis = {
        .x = (f32) dst_fb_region.start_x + corner_radii.top_left_px,
        .y = (f32) dst_fb_region.start_y + corner_radii.top_left_px,
    };

    f32x2 top_right_axis = {
        .x = (f32) dst_fb_region.start_x + (f32) dst_fb_region.width - corner_radii.top_right_px,
        .y = (f32) dst_fb_region.start_y + corner_radii.top_right_px,
    };

    f32x2 bottom_left_axis = {
        .x = (f32) dst_fb_region.start_x + corner_radii.bottom_left_px,
        .y = (f32) dst_fb_region.start_y + (f32) dst_fb_region.height - corner_radii.bottom_left_px,
    };

    f32x2 bottom_right_axis = {
        .x = (f32) dst_fb_region.start_x + (f32) dst_fb_region.width - corner_radii.bottom_right_px,
        .y = (f32) dst_fb_region.start_y + (f32) dst_fb_region.height - corner_radii.bottom_right_px,
    };

    switch (cmd->mode) {
        case RENDER_CMD_DRAW_RECT_MODE_FILL: {
            rgba8 color = cmd->data.fill.color;

            for (i32 dst_y_idx = dst_start_y; dst_y_idx < dst_end_y; dst_y_idx++) {
                for (i32 dst_x_idx = dst_start_x; dst_x_idx < dst_end_x; dst_x_idx++) {
                    f32x2 px_pos = {.x = (f32) dst_x_idx + 0.5f, .y = (f32) dst_y_idx + 0.5f};
                    if (!not_rounded
                        && px_beyond_corner_axes(
                            px_pos,
                            top_left_axis,
                            top_right_axis,
                            bottom_left_axis,
                            bottom_right_axis
                        )
                        && px_outside_rounded_corner(
                            px_pos,
                            top_left_axis,
                            top_right_axis,
                            bottom_left_axis,
                            bottom_right_axis,
                            corner_radii
                        )
                    ) {
                        continue;
                    }

                    i64 px_idx = dst_y_idx * dst_fb_pitch_bytes + dst_x_idx * dst_px_size;

                    pixel_set(
                        &(Pixel){
                            .format = dst_px_format,
                            .start = dst_fb_bytes + px_idx
                        },
                        &(Pixel){
                            .format = PIXEL_FORMAT_RGBA8,
                            .start = color.data
                        }
                    );
                }
            }

            break;
        }
        case RENDER_CMD_DRAW_RECT_MODE_BORDER: {
            rgba8 color = cmd->data.border.color;
            RenderCmdDrawRectBorderWidths widths = cmd->data.border.widths;
            f32x4 region = {
                .start_x = (f32) dst_start_x,
                .start_y = (f32) dst_start_y,
                .width = (f32) (dst_end_x - dst_start_x),
                .height = (f32) (dst_end_y - dst_start_y)
            };
            for (i32 dst_y_idx = dst_start_y; dst_y_idx < dst_end_y; dst_y_idx++) {
                for (i32 dst_x_idx = dst_start_x; dst_x_idx < dst_end_x; dst_x_idx++) {
                    i64 px_idx = dst_y_idx * dst_fb_pitch_bytes + dst_x_idx * dst_px_size;

                    Pixel dst_px = {
                        .format = dst_px_format,
                        .start = dst_fb_bytes + px_idx,
                    };
                    Pixel src_px = {
                        .format = PIXEL_FORMAT_RGBA8,
                        .start = color.data
                    };

                    f32x2 px_pos = {.x = (f32) dst_x_idx + 0.5f, .y = (f32) dst_y_idx + 0.5f};
                    if (!not_rounded) {
                        if ((
                                !px_beyond_corner_axes(
                                    px_pos, top_left_axis,
                                    top_right_axis,
                                    bottom_left_axis,
                                    bottom_right_axis)
                                && px_within_straight_border(px_pos, region, widths)
                            )
                            || px_within_rounded_border(
                                px_pos,
                                top_left_axis,
                                top_right_axis,
                                bottom_left_axis,
                                bottom_right_axis,
                                corner_radii,
                                widths
                            )) {
                            pixel_set(&dst_px, &src_px);
                        }
                    } else {
                        if (px_within_straight_border(px_pos, region, widths)) {
                            pixel_set(&dst_px, &src_px);
                        }
                    }
                }
            }

            break;
        }
        case RENDER_CMD_DRAW_RECT_MODE_BLIT: {
            auto blit_data = &cmd->data.blit;
            for (i32 dst_y_idx = dst_start_y; dst_y_idx < dst_end_y; dst_y_idx++) {
                for (i32 dst_x_idx = dst_start_x; dst_x_idx < dst_end_x; dst_x_idx++) {
                    i64 px_idx = dst_y_idx * dst_fb_pitch_bytes + dst_x_idx * dst_px_size;

                    f32 dst_y_pct = (f32) (dst_y_idx - dst_start_y) / (f32) (dst_end_y - dst_start_y);
                    f32 dst_x_pct = (f32) (dst_x_idx - dst_start_x) / (f32) (dst_end_x - dst_start_x);
                    rgba32f sample = framebuffer_get_sample(
                        blit_data->src,
                        (f32x2){.x = dst_x_pct, .y = dst_y_pct},
                        (FramebufferSampleInfo){}
                    );

                    pixel_set(
                        &(Pixel){
                            .format = dst_px_format,
                            .start = dst_fb_bytes + px_idx,
                        },
                        &(Pixel){
                            .format = PIXEL_FORMAT_RGBA32F,
                            .start = (u8 *) sample.data
                        }
                    );
                }
            }

            break;
        }
        default:
            crash_msg("Unhandled draw rect mode\n");
            break;
    }

    /*
    RenderCmdDrawRectBorderWidths border_widths = data->border_widths;
    rgba8 bg_color = data->bg_color;
    rgba8 border_color = data->border_color;

    Pixel bg_px = {
        .format = PIXEL_FORMAT_RGBA8,
        .start = (u8 *) &bg_color,
    };

    Pixel border_px = {
        .format = PIXEL_FORMAT_RGBA8,
        .start = (u8 *) &border_color,
    };


    for (i32 dst_y_idx = dst_start_y; dst_y_idx < dst_end_y; dst_y_idx++) {
        for (i32 dst_x_idx = dst_start_x; dst_x_idx < dst_end_x; dst_x_idx++) {
            u8 *dst_px_start = dst_fb_bytes + dst_y_idx * dst_fb_pitch_bytes + dst_x_idx * dst_px_size;

            Pixel dst_px = {
                .format = dst_px_format,
                .start = dst_px_start,
            };

            f32x2 dst_px_coord = {
                .x = (f32) dst_x_idx,
                .y = (f32) dst_y_idx,
            };

            Pixel src_px = {};

            if (data->src_blit.pixel_bytes) {
                f32 dst_y_pct = (f32) (dst_y_idx - dst_start_y) / (f32) (dst_end_y - dst_start_y);
                f32 dst_x_pct = (f32) (dst_x_idx - dst_start_x) / (f32) (dst_end_x - dst_start_x);

                i32 src_y_idx = (i32) (dst_y_pct * (f32) data->src_blit.size.height);
                i32 src_x_idx = (i32) (dst_x_pct * (f32) data->src_blit.size.width);

                src_px.start = data->src_blit.pixel_bytes
                               + src_y_idx * data->src_blit.pitch_bytes
                               + src_x_idx * pixel_size(data->src_blit.px_format);

                src_px.format = data->src_blit.px_format;
            }

            if (dst_y_idx <= top_left_axis.y && dst_x_idx <= top_left_axis.x) {
                f32x2 top_left_axis_coord = itof32x2(top_left_axis);
                f32 dist = f32x2_dist(
                    top_left_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.top_left_px) {
                    pixel_set(&dst_px, &bg_px);

                    if (src_px.start) {
                        pixel_set(&dst_px, &src_px);
                    }

                    f32x2 vec = f32x2_sub(dst_px_coord, top_left_axis_coord);
                    if (vec.y < 0) {
                        vec.y *= -1;
                    }
                    if (vec.x != 0 || vec.y != 0) {
                        f32 x_axis_rad = atan2f(vec.y, vec.x);
                        if (x_axis_rad < 0) {
                            x_axis_rad *= -1;
                        }
                        f32 lerp = (x_axis_rad - (f32) M_PI_2) / (f32) M_PI_2;
                        f32 lerp_border_width = f32_lerp(border_widths.top_px, border_widths.left_px, lerp);
                        if (corner_radii.top_left_px - dist <= lerp_border_width) {
                            pixel_set(&dst_px, &border_px);
                        }
                    }
                }
            } else if (dst_y_idx <= top_right_axis.y && dst_x_idx >= top_right_axis.x) {
                f32x2 top_right_axis_coord = itof32x2(top_right_axis);

                f32 dist = f32x2_dist(
                    top_right_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.top_right_px) {
                    pixel_set(&dst_px, &bg_px);

                    if (src_px.start) {
                        pixel_set(&dst_px, &src_px);
                    }

                    f32x2 vec = f32x2_sub(dst_px_coord, top_right_axis_coord);
                    if (vec.y < 0) {
                        vec.y *= -1;
                    }

                    if (vec.x != 0 || vec.y != 0) {
                        f32 x_axis_rad = atan2f(vec.y, vec.x);
                        if (x_axis_rad < 0) {
                            x_axis_rad *= -1;
                        }
                        f32 lerp = x_axis_rad / (f32) M_PI_2;
                        f32 lerp_border_width = f32_lerp(border_widths.right_px, border_widths.top_px, lerp);
                        if (corner_radii.top_right_px - dist <= lerp_border_width) {
                            pixel_set(&dst_px, &border_px);
                        }
                    }
                }
            } else if (dst_y_idx >= bottom_left_axis.y && dst_x_idx <= bottom_left_axis.x) {
                f32x2 bottom_left_axis_coord = itof32x2(bottom_left_axis);

                f32 dist = f32x2_dist(
                    bottom_left_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.bottom_left_px) {
                    pixel_set(&dst_px, &bg_px);

                    if (src_px.start) {
                        pixel_set(&dst_px, &src_px);
                    }

                    f32x2 vec = f32x2_sub(dst_px_coord, bottom_left_axis_coord);
                    if (vec.y > 0) {
                        vec.y *= -1;
                    }
                    if (vec.x != 0 || vec.y != 0) {
                        f32 x_axis_rad = atan2f(vec.y, vec.x);
                        if (x_axis_rad > 0) {
                            x_axis_rad *= -1;
                        }
                        f32 lerp = (x_axis_rad + (f32) M_PI) / (f32) M_PI_2;
                        f32 lerp_border_width = f32_lerp(border_widths.left_px, border_widths.bottom_px, lerp);
                        if (corner_radii.bottom_left_px - dist <= lerp_border_width) {
                            pixel_set(&dst_px, &border_px);
                        }
                    }
                }
            } else if (dst_y_idx >= bottom_right_axis.y && dst_x_idx >= bottom_right_axis.x) {
                f32x2 bottom_right_axis_coord = itof32x2(bottom_right_axis);

                f32 dist = f32x2_dist(
                    bottom_right_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.bottom_right_px) {
                    pixel_set(&dst_px, &bg_px);

                    if (src_px.start) {
                        pixel_set(&dst_px, &src_px);
                    }

                    f32x2 vec = f32x2_sub(dst_px_coord, bottom_right_axis_coord);

                    if (vec.y > 0) {
                        vec.y *= -1;
                    }

                    if (vec.x != 0 || vec.y != 0) {
                        f32 x_axis_rad = atan2f(vec.y, vec.x);
                        if (x_axis_rad > 0) {
                            x_axis_rad *= -1;
                        }

                        f32 lerp = x_axis_rad / (f32) -M_PI_2;
                        f32 lerp_border_width = f32_lerp(border_widths.right_px, border_widths.bottom_px, lerp);
                        if (corner_radii.bottom_left_px - dist <= lerp_border_width) {
                            pixel_set(&dst_px, &border_px);
                        }
                    }
                }
            } else {
                pixel_set(&dst_px, &bg_px);

                if (src_px.start) {
                    pixel_set(&dst_px, &src_px);
                }

                if (dst_y_idx - dst_start_y < (i32) border_widths.top_px
                    || dst_end_y - dst_y_idx <= (i32) border_widths.bottom_px
                    || dst_x_idx - dst_start_x < (i32) border_widths.left_px
                    || dst_end_x - dst_x_idx <= (i32) border_widths.right_px) {
                    pixel_set(&dst_px, &border_px);
                }
            }
        }
    }
    */
}
