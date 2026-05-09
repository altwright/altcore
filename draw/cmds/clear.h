//
// Created by wright on 5/8/26.
//

#ifndef ALTCORE_CLEAR_H
#define ALTCORE_CLEAR_H

#include "../../worker.h"
#include "../framebuffer.h"
#include "../../types.h"

void soft_renderer_clear(FramebufferData fb_data, rgba8888 rgba, Worker* workers[], i32 workers_len);

#endif //ALTCORE_CLEAR_H
