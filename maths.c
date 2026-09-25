//
// Created by wright on 5/25/26.
//

#include "maths.h"
#include "cglm/vec2.h"

f32x44 f32x44_identity() {
    return (f32x44) {
        .data = GLM_MAT4_IDENTITY_INIT
    };
}

f32x2 f32x2_sub(f32x2 left, f32x2 right) {
    return (f32x2) {
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
