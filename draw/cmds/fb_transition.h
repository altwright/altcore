//
// Created by wright on 5/8/26.
//

#ifndef ALTCORE_RESOLVE_H
#define ALTCORE_RESOLVE_H

#include "../framebuffer.h"
#include "../../worker.h"
#include "../../types.h"

void soft_renderer_fb_transition(Framebuffer* fb, Worker** workers, i32 workers_len);

#endif //ALTCORE_RESOLVE_H
