//
// Created by wright on 6/8/26.
//

#ifndef ALTCORE_LIGHTS_H
#define ALTCORE_LIGHTS_H

#include "../types.h"
#include "pixels.h"

typedef struct POINT_LIGHT_T {
    f32x3 position;
    f32 fade_start_radius;
    f32 fade_end_radius;
    RGBA8888 color;
} PointLight;

#endif //ALTCORE_LIGHTS_H
