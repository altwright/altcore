//
// Created by wright on 10/18/26.
//

#include "draw_model.h"
#include "../framebuffer.h"
#include "../framebuffer.impl.h"
#include "../vertex_buffer.impl.h"
#include "../index_buffer.impl.h"
#include "../../debug.h"
#include "../../maths.h"

constexpr f32 kEpsilon = 0.0001f;

static f32x3 to_screen_space(f32x4 clip_space, i32x2 screen_size) {
    f32 inv_w = 1.0f / clip_space.w;
    return (f32x3){
        .x = (clip_space.x * inv_w * 0.5f + 0.5f) * (f32) screen_size.width,
        .y = (clip_space.y * inv_w * 0.5f + 0.5f) * (f32) screen_size.height,
        .z = clip_space.z * inv_w,
    };
}

static f32 screen_triangle_area_2(f32x2 a, f32x2 b, f32x2 c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static bool tri_face_culled(f32x4 clip_space_tri[3], RenderCmdDrawModelCullMode cull_mode) {
    bool culled = false;


    switch (cull_mode) {
        case RENDER_CMD_DRAW_MODEL_CULL_MODE_NONE: {
            break;
        }
        default:
            crash_msg("Unhandled cull mode %d\n", cull_mode);
            break;
    }

    return culled;
}

void soft_cmd_draw_model(RenderCmdDrawModel *data) {
    FramebufferInfo dst_fb_info = framebuffer_get_info(data->dst_framebuffer);
    if (dst_fb_info.type != FRAMEBUFFER_TYPE_PIXEL) {
        crash_msg("Unhandled framebuffer type %d\n", dst_fb_info.type);
    }

    i32x2 dst_fb_size = dst_fb_info.data.pixel_buf.size;
    i64 dst_fb_pitch = dst_fb_info.data.pixel_buf.pitch_bytes;
    PixelFormat dst_fb_pixel_format = dst_fb_info.data.pixel_buf.format;
    i64 dst_fb_pixel_format_size = pixel_size(dst_fb_pixel_format);
    f32x4 dst_fb_scissor = itof32x4(dst_fb_info.data.pixel_buf.scissor);

    u8 *dst_fb_bytes = framebuffer_impl_get_bytes(data->dst_framebuffer);

    for (i64 mesh_idx = 0; mesh_idx < data->model->meshes.len; mesh_idx++) {
        Mesh *mesh = ARRAY_GET(&data->model->meshes, mesh_idx);
        for (i64 prim_idx = 0; prim_idx < mesh->prims.len; prim_idx++) {
            MeshPrimitive *prim = ARRAY_GET(&mesh->prims, prim_idx);

            const u16 *indices = nullptr;
            i64 indices_len = 0;

            index_buffer_impl_get_indices(prim->indices, &indices, &indices_len);

            data->shader_data_init_fn(data->model, mesh_idx, prim_idx, data->shader_data.bytes);

            switch (prim->type) {
                case MESH_PRIMITIVE_TYPE_TRIANGLES: {
                    if (indices_len % 3 != 0) {
                        crash_msg("Expected indices length %d to be a multiple of 3\n", indices_len);
                    }

                    for (i64 start_indice_idx = 0; start_indice_idx < indices_len; start_indice_idx += 3) {
                        i64 triangle_indices[3] = {};
                        f32x4 clip_space_triangle[3] = {};
                        for (i64 vert_idx = 0; vert_idx < 3; vert_idx++) {
                            triangle_indices[vert_idx] = indices[start_indice_idx + vert_idx];
                            clip_space_triangle[vert_idx] = data->
                                    vert_shader_fn(triangle_indices[vert_idx], data->shader_data.bytes);
                            clip_space_triangle[vert_idx].y *= -1;
                        }

                        // TODO: Handle tri passing through near plane
                        if (clip_space_triangle[0].w <= kEpsilon
                            || clip_space_triangle[1].w <= kEpsilon
                            || clip_space_triangle[2].w <= kEpsilon) {
                            continue;
                        }

                        f32 clip_space_tri_inv_w[3] = {};
                        f32x3 screen_space_3d_tri[3] = {};
                        f32x2 screen_space_2d_tri[3] = {};
                        for (i64 vert_idx = 0; vert_idx < 3; vert_idx++) {
                            clip_space_tri_inv_w[vert_idx] = 1 / clip_space_triangle[vert_idx].w;
                            screen_space_3d_tri[vert_idx] = to_screen_space(clip_space_triangle[vert_idx], dst_fb_size);
                            screen_space_2d_tri[vert_idx] = f32x3to2(screen_space_3d_tri[vert_idx]);
                        }

                        f32 screen_space_triangle_area_2 = screen_triangle_area_2(
                            screen_space_2d_tri[0],
                            screen_space_2d_tri[1],
                            screen_space_2d_tri[2]
                        );

                        if (ABS(screen_space_triangle_area_2) <= 2 * kEpsilon) {
                            continue;
                        }

                        bool face_culled = false;
                        switch (data->cull_mode) {
                            case RENDER_CMD_DRAW_MODEL_CULL_MODE_NONE: {
                                break;
                            }
                            case RENDER_CMD_DRAW_MODEL_CULL_MODE_BACK: {
                                if (screen_space_triangle_area_2 > 0) {
                                    face_culled = true;
                                }
                                break;
                            }
                            case RENDER_CMD_DRAW_MODEL_CULL_MODE_FRONT: {
                                if (screen_space_triangle_area_2 < 0) {
                                    face_culled = true;
                                }
                                break;
                            }
                            default:
                                crash_msg("Unhandled cull mode %d\n", data->cull_mode);
                                break;
                        }

                        if (face_culled) {
                            continue;
                        }

                        f32 min_x = floorf(
                            MIN(screen_space_2d_tri[0].x,
                                MIN(
                                    screen_space_2d_tri[1].x,
                                    screen_space_2d_tri[2].x)
                            )
                        );
                        f32 max_x = ceilf(
                            MAX(screen_space_2d_tri[0].x,
                                MAX(
                                    screen_space_2d_tri[1].x,
                                    screen_space_2d_tri[2].x
                                )
                            )
                        );
                        f32 min_y = floorf(
                            MIN(screen_space_2d_tri[0].y,
                                MIN(
                                    screen_space_2d_tri[1].y,
                                    screen_space_2d_tri[2].y)
                            )
                        );
                        f32 max_y = ceilf(
                            MAX(screen_space_2d_tri[0].y,
                                MAX(
                                    screen_space_2d_tri[1].y,
                                    screen_space_2d_tri[2].y
                                )
                            )
                        );

                        i32x4 bbox = ftoi32x4(f32x4_intersect(
                            dst_fb_scissor,
                            (f32x4){
                                .start_x = min_x, .start_y = min_y, .width = max_x - min_x, .height = max_y - min_y
                            }
                        ));

                        for (i32 y_idx = bbox.start_y; y_idx < bbox.start_y + bbox.height; y_idx++) {
                            for (i32 x_idx = bbox.start_x; x_idx < bbox.start_x + bbox.width; x_idx++) {
                                f32x2 px = {
                                    .x = (f32) x_idx + 0.5f,
                                    .y = (f32) y_idx + 0.5f,
                                };

                                // TODO: Can compute top-left corner once and add the px step
                                f32 edge_0_to_p_area_2 = screen_triangle_area_2(
                                    screen_space_2d_tri[1], screen_space_2d_tri[2], px);
                                f32 edge_1_to_p_area_2 = screen_triangle_area_2(
                                    screen_space_2d_tri[2], screen_space_2d_tri[0], px);
                                f32 edge_2_to_p_area_2 = screen_triangle_area_2(
                                    screen_space_2d_tri[0], screen_space_2d_tri[1], px);

                                bool all_negative =
                                        edge_0_to_p_area_2 < 0 && edge_1_to_p_area_2 < 0 && edge_2_to_p_area_2 < 0;
                                bool all_positive =
                                        edge_0_to_p_area_2 > 0 && edge_1_to_p_area_2 > 0 && edge_2_to_p_area_2 > 0;
                                bool px_inside = false;

                                switch (data->cull_mode) {
                                    case RENDER_CMD_DRAW_MODEL_CULL_MODE_NONE: {
                                        if (all_negative || all_positive) {
                                            px_inside = true;
                                        }
                                        break;
                                    }
                                    case RENDER_CMD_DRAW_MODEL_CULL_MODE_BACK: {
                                        if (all_negative) {
                                            px_inside = true;
                                        }
                                        break;
                                    }
                                    case RENDER_CMD_DRAW_MODEL_CULL_MODE_FRONT: {
                                        if (all_positive) {
                                            px_inside = true;
                                        }
                                        break;
                                    }
                                    default:
                                        crash_msg("Unhandled cull mode %d\n", data->cull_mode);
                                        break;
                                }

                                if (!px_inside) {
                                    continue;
                                }

                                f32x3 bary = {
                                    .u = edge_0_to_p_area_2 / screen_space_triangle_area_2,
                                    .v = edge_1_to_p_area_2 / screen_space_triangle_area_2,
                                    .w = edge_2_to_p_area_2 / screen_space_triangle_area_2,
                                };

                                f32x3 frag = {
                                    .x = px.x,
                                    .y = px.y,
                                    .z = bary.u * screen_space_3d_tri[0].z
                                         + bary.v * screen_space_3d_tri[1].z
                                         + bary.w * screen_space_3d_tri[2].z,
                                };

                                f32 bary_inv_w = bary.u * clip_space_tri_inv_w[0]
                                                 + bary.v * clip_space_tri_inv_w[1]
                                                 + bary.w * clip_space_tri_inv_w[2];

                                for (i64 vert_idx = 0; vert_idx < 3; vert_idx++) {
                                    bary.data[vert_idx] *= clip_space_tri_inv_w[vert_idx] / bary_inv_w;
                                }

                                rgba32f color = data->frag_shader_fn(
                                    frag,
                                    bary,
                                    data->shader_data.bytes
                                );

                                pixel_set(
                                    &(Pixel){
                                        .format = dst_fb_pixel_format,
                                        .start = dst_fb_bytes + (y_idx * dst_fb_pitch + x_idx *
                                                                 dst_fb_pixel_format_size),
                                    },
                                    &(Pixel){
                                        .format = PIXEL_FORMAT_RGBA32F,
                                        .start = (u8 *) color.data,
                                    }
                                );
                            }
                        }
                    }

                    break;
                }
                default:
                    crash_msg("Unhandled primitive type %d\n", prim->type);
                    break;
            }
        }
    }
}
