//
// Created by wright on 5/25/26.
//

#include "maths.h"

f32x44 f32x44_identity() {
    return (f32x44) {
        .data = GLM_MAT4_IDENTITY_INIT
    };
}
