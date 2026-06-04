//
// Created by wright on 5/8/26.
//

#ifndef ALTCORE_CLEAR_H
#define ALTCORE_CLEAR_H

#include "../../types.h"
#include "../pixels.h"
#include "../framebuffer.h"

void soft_cmd_clear(
    Framebuffer* px_buf,
    RGBA8888 rgba
);

#endif //ALTCORE_CLEAR_H
