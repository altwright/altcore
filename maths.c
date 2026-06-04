//
// Created by wright on 5/25/26.
//

#include "maths.h"

f32 f32_min(f32 left, f32 right) {
    float *fl = (float*)&left;
    float *fr = (float*)&right;
    return *fl < *fr ? left : right;
}

f32 f32_max(f32 left, f32 right) {
    float *fl = (float*)&left;
    float *fr = (float*)&right;
    return *fl > *fr ? left : right;
}

f32 f32_clamp(f32 val, f32 min, f32 max) {
    float *fval = (float*)&val;
    float *fmin = (float*)&min;
    float *fmax = (float*)&max;

    return (
        *fval < *fmin
        ? min
        : (
            *fval > *fmax ? max : val
        )
    );
}

f32 f32_saturate(f32 val) {
    return f32_clamp(val, F32(0), F32(1));
}

f64 f64_min(f64 left, f64 right) {
    double *dl = (double*)&left;
    double *dr = (double*)&right;

    return *dl < *dr ? left : right;
}

f64 f64_max(f64 left, f64 right) {
    double *dl = (double*)&left;
    double *dr = (double*)&right;

    return *dl > *dr ? left : right;
}

f64 f64_clamp(f64 val, f64 min, f64 max) {
    double *dval = (double*)&val;
    double *dmin = (double*)&min;
    double *dmax = (double*)&max;

    return (
        *dval < *dmin
        ? min
        : (
            *dval > *dmax ? max : val
        )
    );
}

f64 f64_saturate(f64 val) {
    return f64_clamp(val, F64(0), F64(1));
}
