//
// Created by wright on 5/8/26.
//

#ifndef ALTCORE_CLEAR_H
#define ALTCORE_CLEAR_H

#include "../../types.h"
#include "../pixels.h"

void cmd_soft_single_clear(u8* pixel_bytes, PixelFormat pixel_format, i32x2 size, i64 pitch_bytes, RGBA8888 rgba);

#endif //ALTCORE_CLEAR_H
