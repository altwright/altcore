//
// Created by wright on 5/25/26.
//

#include "draw_rect.h"

#include <assert.h>

#include "../../maths.h"
#include "../framebuffer.impl.h"

void soft_cmd_draw_rect(
    Framebuffer *px_buf,
    i32x4 px_buf_dst,
    RGBA8888 bg_color,
    RectCornerRadii corner_radii,
    RGBA8888 border_color,
    RectBorderWidths border_widths
) {
    FramebufferInfo px_buf_info = framebuffer_get_info(px_buf);
    assert(px_buf_info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8 *px_buf_bytes = framebuffer_impl_get_bytes(px_buf);
    PixelFormat px_format = px_buf_info.data.pixel_buf.format;
    i32x2 px_buf_size = px_buf_info.data.pixel_buf.size;
    i64 px_buf_pitch_bytes = px_buf_info.data.pixel_buf.pitch_bytes;

    PixelColor bg_px = pixels_convert_rgba(px_format, bg_color);
    PixelColor border_px = pixels_convert_rgba(px_format, border_color);
    i32 px_size = pixels_get_size(px_format);

    i32 start_y = CLAMP(px_buf_dst.start_y, 0, px_buf_size.height);
    i32 end_y = CLAMP(px_buf_dst.start_y + px_buf_dst.height, start_y, px_buf_size.height);
    i32 start_x = CLAMP(px_buf_dst.start_x, 0, px_buf_size.width);
    i32 end_x = CLAMP(px_buf_dst.start_x + px_buf_dst.width, start_x, px_buf_size.width);

    i32x2 top_left_axis = {
        .x = px_buf_dst.start_x + (i32) corner_radii.top_left_px,
        .y = px_buf_dst.start_y + (i32) corner_radii.top_left_px,
    };

    i32x2 top_right_axis = {
        .x = px_buf_dst.start_x + px_buf_dst.width - (i32) corner_radii.top_right_px,
        .y = px_buf_dst.start_y + (i32) corner_radii.top_right_px,
    };

    i32x2 bottom_left_axis = {
        .x = px_buf_dst.start_x + (i32) corner_radii.bottom_left_px,
        .y = px_buf_dst.start_y + px_buf_dst.height - (i32) corner_radii.bottom_left_px,
    };

    i32x2 bottom_right_axis = {
        .x = px_buf_dst.start_x + px_buf_dst.width - (i32) corner_radii.bottom_right_px,
        .y = px_buf_dst.start_y + px_buf_dst.height - (i32) corner_radii.bottom_right_px,
    };

    for (i32 y_idx = start_y; y_idx < end_y; y_idx++) {
        for (i32 x_idx = start_x; x_idx < end_x; x_idx++) {
            u8 *pixel_start = px_buf_bytes + y_idx * px_buf_pitch_bytes + x_idx * px_size;

            f32x2 px_coord = {
                .x = (f32) x_idx,
                .y = (f32) y_idx,
            };

            if (bg_color.a > 0) {
                if (y_idx <= top_left_axis.y && x_idx <= top_left_axis.x) {
                    f32 dist = f32x2_dist(
                        itof32x2(top_left_axis),
                        px_coord
                    );

                    if (dist <= corner_radii.top_left_px) {
                        pixels_set(pixel_start, px_format, bg_px);
                    }
                } else if (y_idx <= top_right_axis.y && x_idx >= top_right_axis.x) {
                    f32 dist = f32x2_dist(
                        itof32x2(top_right_axis),
                        px_coord
                    );

                    if (dist <= corner_radii.top_right_px) {
                        pixels_set(pixel_start, px_format, bg_px);
                    }
                } else if (y_idx >= bottom_left_axis.y && x_idx <= bottom_left_axis.x) {
                    f32 dist = f32x2_dist(
                        itof32x2(bottom_left_axis),
                        px_coord
                    );

                    if (dist <= corner_radii.bottom_left_px) {
                        pixels_set(pixel_start, px_format, bg_px);
                    }
                } else if (y_idx >= bottom_right_axis.y && x_idx >= bottom_right_axis.x) {
                    f32 dist = f32x2_dist(
                        itof32x2(bottom_right_axis),
                        px_coord
                    );

                    if (dist <= corner_radii.bottom_right_px) {
                        pixels_set(pixel_start, px_format, bg_px);
                    }
                } else {
                    pixels_set(pixel_start, px_format, bg_px);
                }
            }

            if (border_color.a > 0) {
                if (y_idx <= top_left_axis.y && x_idx <= top_left_axis.x) {
                    f32x2 top_left_axis_coord = itof32x2(top_left_axis);

                    f32 dist = f32x2_dist(
                        top_left_axis_coord,
                        px_coord
                    );

                    if (dist <= corner_radii.top_left_px) {
                        f32x2 vec = f32x2_sub(px_coord, top_left_axis_coord);
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
                                pixels_set(pixel_start, px_format, border_px);
                            }
                        }
                    }
                } else if (y_idx <= top_right_axis.y && x_idx >= top_right_axis.x) {
                    f32x2 top_right_axis_coord = itof32x2(top_right_axis);

                    f32 dist = f32x2_dist(
                        top_right_axis_coord,
                        px_coord
                    );

                    if (dist <= corner_radii.top_right_px) {
                        f32x2 vec = f32x2_sub(px_coord, top_right_axis_coord);
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
                                pixels_set(pixel_start, px_format, border_px);
                            }
                        }
                    }
                } else if (y_idx >= bottom_left_axis.y && x_idx <= bottom_left_axis.x) {
                    f32x2 bottom_left_axis_coord = itof32x2(bottom_left_axis);

                    f32 dist = f32x2_dist(bottom_left_axis_coord, px_coord);

                    if (dist <= corner_radii.bottom_left_px) {
                        f32x2 vec = f32x2_sub(px_coord, bottom_left_axis_coord);
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
                                pixels_set(pixel_start, px_format, border_px);
                            }
                        }
                    }
                } else if (y_idx >= bottom_right_axis.y && x_idx >= bottom_right_axis.x) {
                    f32x2 bottom_right_axis_coord = itof32x2(bottom_right_axis);

                    f32 dist = f32x2_dist(bottom_right_axis_coord, px_coord);

                    if (dist <= corner_radii.bottom_right_px) {
                        f32x2 vec = f32x2_sub(px_coord, bottom_right_axis_coord);

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
                                pixels_set(pixel_start, px_format, border_px);
                            }
                        }
                    }
                } else if (y_idx - start_y < (i32) border_widths.top_px
                           || end_y - y_idx <= (i32) border_widths.bottom_px
                           || x_idx - start_x < (i32) border_widths.left_px
                           || end_x - x_idx <= (i32) border_widths.right_px) {
                    pixels_set(pixel_start, px_format, border_px);
                }
            }
        }
    }
}
