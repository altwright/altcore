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

typedef struct F32X44_T {
    union {
        mat4s mat;
        mat4 data;
    };
} f32x44;

f32 f32_lerp(f32 start, f32 end, f32 lerp);

f32x44 f32x44_identity();

f32x2 f32x2_sub(f32x2 left, f32x2 right);

f32 f32x2_dist(f32x2 start, f32x2 end);

#endif //ALTCORE_MATHS_H
