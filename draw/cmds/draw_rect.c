//
// Created by wright on 5/25/26.
//

#include "draw_rect.h"

#include <assert.h>

#include "../../maths.h"
#include "../framebuffer.impl.h"
#include "../../debug.h"

void soft_cmd_draw_rect(RenderCmdDrawRect *data) {
    FramebufferInfo dst_fb_info = framebuffer_get_info(data->dst_framebuffer);
    if (dst_fb_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Destination framebuffer is not a pixel buffer\n");
    }

    u8 *dst_fb_bytes = framebuffer_impl_get_bytes(data->dst_framebuffer);
    PixelFormat dst_px_format = dst_fb_info.data.pixel_buf.format;
    i64 dst_px_size = pixel_size(dst_px_format);
    i32x2 dst_fb_size = dst_fb_info.data.pixel_buf.size;
    i64 dst_fb_pitch_bytes = dst_fb_info.data.pixel_buf.pitch_bytes;
    i32x4 dst_fb_region = ftoi32x4(data->dst_region);
    RectCornerRadii corner_radii = data->corner_radii;
    RectBorderWidths border_widths = data->border_widths;
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

    i32 dst_start_y = CLAMP(dst_fb_region.start_y, 0, dst_fb_size.height);
    i32 dst_end_y = CLAMP(dst_fb_region.start_y + dst_fb_region.height, dst_start_y, dst_fb_size.height);
    i32 dst_start_x = CLAMP(dst_fb_region.start_x, 0, dst_fb_size.width);
    i32 dst_end_x = CLAMP(dst_fb_region.start_x + dst_fb_region.width, dst_start_x, dst_fb_size.width);

    i32x2 top_left_axis = {
        .x = dst_fb_region.start_x + (i32) corner_radii.top_left_px,
        .y = dst_fb_region.start_y + (i32) corner_radii.top_left_px,
    };

    i32x2 top_right_axis = {
        .x = dst_fb_region.start_x + dst_fb_region.width - (i32) corner_radii.top_right_px,
        .y = dst_fb_region.start_y + (i32) corner_radii.top_right_px,
    };

    i32x2 bottom_left_axis = {
        .x = dst_fb_region.start_x + (i32) corner_radii.bottom_left_px,
        .y = dst_fb_region.start_y + dst_fb_region.height - (i32) corner_radii.bottom_left_px,
    };

    i32x2 bottom_right_axis = {
        .x = dst_fb_region.start_x + dst_fb_region.width - (i32) corner_radii.bottom_right_px,
        .y = dst_fb_region.start_y + dst_fb_region.height - (i32) corner_radii.bottom_right_px,
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

                src_px.start= data->src_blit.pixel_bytes
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
}
