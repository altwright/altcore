//
// Created by wright on 5/25/26.
//

#include "maths.h"

#include "cglm/cam.h"
#include "cglm/vec2.h"
#include "cglm/affine.h"

f32x2 f32x2_add(f32x2 left, f32x2 right) {
    return (f32x2){
        .x = left.x + right.x,
        .y = left.y + right.y,
    };
}

f32x2 f32x2_sub(f32x2 left, f32x2 right) {
    return (f32x2){
        .x = left.x - right.x,
        .y = left.y - right.y
    };
}

f32x2 f32x2_scale(f32x2 vec, f32 scale) {
    return (f32x2){
        .x = vec.x * scale,
        .y = vec.y * scale,
    };
}

f32 f32x2_dist(f32x2 start, f32x2 end) {
    return glm_vec2_distance(start.data, end.data);
}

f32x3 f32x3_add(f32x3 left, f32x3 right) {
    return (f32x3){
        .x = left.x + right.x,
        .y = left.y + right.y,
        .z = left.z + right.z,
    };
}

f32x3 f32x3_scale(f32x3 vec, f32 scale) {
    return (f32x3){
        .x = vec.x * scale,
        .y = vec.y * scale,
        .z = vec.z * scale,
    };
}

f32 f32x3_mag(f32x3 vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

f32x3 f32x3_unit(f32x3 vec) {
    return f32x3_scale(vec, 1 / f32x3_mag(vec));
}

f32 f32x3_dot(f32x3 left, f32x3 right) {
    return glm_vec3_dot(left.data, right.data);
}

f32x16 f32x16_matrix_id() {
    return (f32x16){
        .matrix = {
            .raw = GLM_MAT4_IDENTITY_INIT
        },
    };
}

f32x16 f32x16_matrix_translate(f32x16 orig, f32x3 delta) {
    f32x16 out = orig;
    glm_translate(out.matrix.raw, delta.data);
    return out;
}

f32x16 f32x16_matrix_look(f32x3 eye, f32x3 dir, f32x3 up) {
    f32x16 out = {};
    glm_look(eye.data, dir.data, up.data, out.matrix.raw);
    return out;
}

f32x16 f32x16_matrix_projection(f32 fov_y_rad, f32 aspect_ratio, f32 near_plane_dist, f32 far_plane_dist) {
    f32x16 out = {};
    glm_perspective(fov_y_rad, aspect_ratio, near_plane_dist, far_plane_dist, out.matrix.raw);
    return out;
}

f32x16 f32x16_matrix_mul(f32x16 left, f32x16 right) {
    f32x16 out = {};
    glm_mat4_mul(left.matrix.raw, right.matrix.raw, out.matrix.raw);
    return out;
}

f32x4 f32x16_matrix_mulv(f32x16 left, f32x4 right) {
    f32x4 out = {};
    glm_mat4_mulv(left.matrix.raw, right.data, out.data);
    return out;
}

f32 f32_lerp(f32 start, f32 end, f32 lerp) {
    lerp = SATURATE(lerp);
    return lerp * end + (1.0f - lerp) * start;
}

f32x4 f32x4_scale(f32x4 src, f32 scale) {
    return (f32x4){
        .x = src.x * scale,
        .y = src.y * scale,
        .z = src.z * scale,
        .w = src.w * scale,
    };
}

f32x4 f32x4_add(f32x4 left, f32x4 right) {
    return (f32x4){
        .x = left.x + right.x,
        .y = left.y + right.y,
        .z = left.z + right.z,
        .w = left.w + right.w,
    };
}

f32x4 f32x4_area_union(f32x4 left, f32x4 right) {
    f32x4 union_area = {};

    f32 start_x = MIN(left.start_x, right.start_x);
    f32 start_y = MIN(left.start_y, right.start_y);
    f32 end_x = MAX(left.start_x + left.width, right.start_x + right.width);
    f32 end_y = MAX(left.start_y + left.height, right.start_y + right.height);

    if (start_x < end_x && start_y < end_y) {
        union_area = (f32x4){
            .start_x = start_x,
            .start_y = start_y,
            .width = end_x - start_x,
            .height = end_y - start_y
        };
    }

    return union_area;
}

f32x4 f32x4_area_intersect(f32x4 left, f32x4 right) {
    f32x4 intersect_area = {};

    f32 start_x = MAX(left.start_x, right.start_x);
    f32 start_y = MAX(left.start_y, right.start_y);
    f32 end_x = MIN(left.start_x + left.width, right.start_x + right.width);
    f32 end_y = MIN(left.start_y + left.height, right.start_y + right.height);

    if (start_x < end_x && start_y < end_y) {
        intersect_area = (f32x4){
            .start_x = start_x,
            .start_y = start_y,
            .width = end_x - start_x,
            .height = end_y - start_y
        };
    }

    return intersect_area;
}

i32x4 i32x4_area_union(i32x4 left, i32x4 right) {
    return ftoi32x4(f32x4_area_union(itof32x4(left), itof32x4(right)));
}

i32x4 i32x4_area_intersect(i32x4 left, i32x4 right) {
    return ftoi32x4(f32x4_area_intersect(itof32x4(left), itof32x4(right)));
}
