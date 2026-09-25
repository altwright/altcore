//
// Created by wright on 9/25/26.
//

#include "blit.h"

#include "../framebuffer.impl.h"
#include "../../debug.h"
#include "../../maths.h"

void soft_cmd_blit(RenderCmdBlit *blit_data) {
    FramebufferInfo dst_fb_info = framebuffer_get_info(blit_data->framebuffer);
    if (dst_fb_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Not a pixel framebuffer\n");
    }

    u8 *dst_fb_bytes = framebuffer_impl_get_bytes(blit_data->framebuffer);
    i32x2 dst_fb_size = dst_fb_info.data.pixel_buf.size;
    PixelFormat dst_fb_px_format = dst_fb_info.data.pixel_buf.format;
    i64 dst_fb_px_size = pixel_size(dst_fb_px_format);
    i64 dst_fb_stride = dst_fb_info.data.pixel_buf.pitch_bytes;
    RectCornerRadii corner_radii = blit_data->dst_corner_radii;

    i32x4 dst_region = ftoi32x4(blit_data->dst);

    i32 dst_start_y = CLAMP(dst_region.start_y, 0, dst_fb_size.height);
    i32 dst_end_y = CLAMP(dst_region.start_y + dst_region.height, dst_start_y, dst_fb_size.height);
    i32 dst_start_x = CLAMP(dst_region.start_x, 0, dst_fb_size.width);
    i32 dst_end_x = CLAMP(dst_region.start_x + dst_region.width, dst_start_x, dst_fb_size.width);

    i64 src_px_size = pixel_size(blit_data->src_px_format);

    i32x2 top_left_axis = {
        .x = dst_region.start_x + (i32) corner_radii.top_left_px,
        .y = dst_region.start_y + (i32) corner_radii.top_left_px,
    };

    i32x2 top_right_axis = {
        .x = dst_region.start_x + dst_region.width - (i32) corner_radii.top_right_px,
        .y = dst_region.start_y + (i32) corner_radii.top_right_px,
    };

    i32x2 bottom_left_axis = {
        .x = dst_region.start_x + (i32) corner_radii.bottom_left_px,
        .y = dst_region.start_y + dst_region.height - (i32) corner_radii.bottom_left_px,
    };

    i32x2 bottom_right_axis = {
        .x = dst_region.start_x + dst_region.width - (i32) corner_radii.bottom_right_px,
        .y = dst_region.start_y + dst_region.height - (i32) corner_radii.bottom_right_px,
    };

    for (i32 dst_y_idx = dst_start_y; dst_y_idx < dst_end_y; dst_y_idx++) {
        for (i32 dst_x_idx = dst_start_x; dst_x_idx < dst_end_x; dst_x_idx++) {
            u8 *dst_px_start = dst_fb_bytes + dst_y_idx * dst_fb_stride + dst_x_idx * dst_fb_px_size;

            f32 dst_y_pct = (f32) (dst_y_idx - dst_start_y) / (f32) (dst_end_y - dst_start_y);
            f32 dst_x_pct = (f32) (dst_x_idx - dst_start_x) / (f32) (dst_end_x - dst_start_x);

            i32 src_y_idx = (i32) (dst_y_pct * (f32) blit_data->src_size.height);
            i32 src_x_idx = (i32) (dst_x_pct * (f32) blit_data->src_size.width);

            u8 *src_px_start = blit_data->src_pixels + src_y_idx * blit_data->src_pitch_bytes + src_x_idx * src_px_size;

            Pixel dst_px = {
                .format = dst_fb_px_format,
                .px_start = dst_px_start,
            };

            Pixel src_px = {
                .format = blit_data->src_px_format,
                .px_start = src_px_start,
            };

            f32x2 dst_px_coord = {
                .x = (f32) dst_x_idx,
                .y = (f32) dst_y_idx,
            };

            if (dst_y_idx <= top_left_axis.y && dst_x_idx <= top_left_axis.x) {
                f32x2 top_left_axis_coord = itof32x2(top_left_axis);
                f32 dist = f32x2_dist(
                    top_left_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.top_left_px) {
                    pixel_set(&dst_px, &src_px);
                }
            } else if (dst_y_idx <= top_right_axis.y && dst_x_idx >= top_right_axis.x) {
                f32x2 top_right_axis_coord = itof32x2(top_right_axis);

                f32 dist = f32x2_dist(
                    top_right_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.top_right_px) {
                    pixel_set(&dst_px, &src_px);
                }
            } else if (dst_y_idx >= bottom_left_axis.y && dst_x_idx <= bottom_left_axis.x) {
                f32x2 bottom_left_axis_coord = itof32x2(bottom_left_axis);

                f32 dist = f32x2_dist(
                    bottom_left_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.bottom_left_px) {
                    pixel_set(&dst_px, &src_px);
                }
            } else if (dst_y_idx >= bottom_right_axis.y && dst_x_idx >= bottom_right_axis.x) {
                f32x2 bottom_right_axis_coord = itof32x2(bottom_right_axis);

                f32 dist = f32x2_dist(
                    bottom_right_axis_coord,
                    dst_px_coord
                );

                if (dist <= corner_radii.bottom_right_px) {
                    pixel_set(&dst_px, &src_px);
                }
            } else {
                pixel_set(&dst_px, &src_px);
            }
        }
    }
}
