//
// Created by wright on 10/16/26.
//

#include "shader.h"

#include "../maths.h"

f32x2 barycentric_f32x2(f32x3 barycentric, f32x2 vert_attrs[3]) {
    f32x2 val = {};

    for (i64 vert_idx = 0; vert_idx < 3; vert_idx++) {
        val = f32x2_add(val, f32x2_scale(vert_attrs[vert_idx], barycentric.data[vert_idx]));
    }

    return val;
}

f32x3 barycentric_f32x3(f32x4 barycentric, f32x3 vert_attrs[3]) {
    f32x3 val = {};

    for (i64 vert_idx = 0; vert_idx < 3; vert_idx++) {
        val = f32x3_add(val, f32x3_scale(vert_attrs[vert_idx], barycentric.data[vert_idx]));
    }

    return val;
}
