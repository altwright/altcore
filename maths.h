//
// Created by wright on 5/25/26.
//

#ifndef ALTCORE_MATHS_H
#define ALTCORE_MATHS_H

#include <math.h>

#include "types.h"
#include "cglm/struct/mat4.h"

#ifndef MIN
#define MIN(left, right) \
    ( \
        (left) < (right) ? (left) : (right) \
    )
#endif

#ifndef MAX
#define MAX(left, right) \
    ( \
        (left) > (right) ? (left) : (right) \
    )
#endif

#ifndef CLAMP
#define CLAMP(val, min, max) \
    ( \
        (val) < (min) \
        ? (min) \
        : ( \
            (val) > (max) ? (max) : (val) \
        ) \
    )
#endif

#ifndef SATURATE
#define SATURATE(val) \
    CLAMP(val, 0, 1)
#endif

#ifndef ABS
#define ABS(val) \
    ( \
        (val) < 0 \
        ? -1 * (val) \
        : (val) \
    )
#endif

typedef union F32X16_U {
    mat4s matrix;
} f32x16;

f32 f32_lerp(f32 start, f32 end, f32 lerp);

f32x2 f32x2_add(f32x2 left, f32x2 right);

f32x2 f32x2_sub(f32x2 left, f32x2 right);

f32x2 f32x2_scale(f32x2 vec, f32 scale);

f32 f32x2_dist(f32x2 start, f32x2 end);

f32 f32x3_dot(f32x3 left, f32x3 right);

f32x16 f32x16_matrix_id();

f32x16 f32x16_matrix_look(f32x3 eye, f32x3 dir, f32x3 up);

f32x16 f32x16_matrix_perspective(f32 fov_y_rad, f32 aspect_ratio, f32 near_plane_dist, f32 far_plane_dist);

f32x16 f32x16_matrix_mul(f32x16 left, f32x16 right);

f32x4 f32x16_matrix_mulv(f32x16 left, f32x4 right);

f32x4 f32x4_scale(f32x4 src, f32 scale);

f32x4 f32x4_add(f32x4 left, f32x4 right);

f32x4 f32x4_union(f32x4 left, f32x4 right);

f32x4 f32x4_intersect(f32x4 left, f32x4 right);

i32x4 i32x4_union(i32x4 left, i32x4 right);

i32x4 i32x4_intersect(i32x4 left, i32x4 right);

#endif //ALTCORE_MATHS_H
