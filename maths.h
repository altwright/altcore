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

#ifndef SATURATE
#define SATURATE(val) \
    CLAMP(val, 0, 1)
#endif

#endif //ALTCORE_MATHS_H
