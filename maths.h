//
// Created by wright on 5/25/26.
//

#ifndef ALTCORE_MATHS_H
#define ALTCORE_MATHS_H

#include <math.h>

#include "types.h"

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

#ifndef F32_PI
#define F32_PI F32(M_PI)
#endif

#ifndef F64_PI
#define F64_PI F64(M_PI)
#endif

f32 f32_min(f32 left, f32 right);

f32 f32_max(f32 left, f32 right);

f32 f32_clamp(f32 val, f32 min, f32 max);

f32 f32_saturate(f32 val);

f64 f64_min(f64 left, f64 right);

f64 f64_max(f64 left, f64 right);

f64 f64_clamp(f64 val, f64 min, f64 max);

f64 f64_saturate(f64 val);

#endif //ALTCORE_MATHS_H
