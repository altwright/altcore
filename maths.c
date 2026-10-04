//
// Created by wright on 5/25/26.
//

#include "maths.h"
#include "cglm/vec2.h"

f32x44 f32x44_identity() {
    return (f32x44){
        .data = GLM_MAT4_IDENTITY_INIT
    };
}

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

f32 f32x2_dist(f32x2 start, f32x2 end) {
    return glm_vec2_distance(start.data, end.data);
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

f32x4 f32x4_union(f32x4 left, f32x4 right) {
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

f32x4 f32x4_intersect(f32x4 left, f32x4 right) {
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

i32x4 i32x4_union(i32x4 left, i32x4 right) {
    return ftoi32x4(f32x4_union(itof32x4(left), itof32x4(right)));
}

i32x4 i32x4_intersect(i32x4 left, i32x4 right) {
    return ftoi32x4(f32x4_intersect(itof32x4(left), itof32x4(right)));
}
