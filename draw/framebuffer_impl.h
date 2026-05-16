//
// Created by wright on 5/16/26.
//

#ifndef ALTCORE_FRAMEBUFFER_IMPL_H
#define ALTCORE_FRAMEBUFFER_IMPL_H

#include "framebuffer.h"

void framebuffer_impl_lock(Framebuffer *framebuffer);

void framebuffer_impl_unlock(Framebuffer *framebuffer);

u8* framebuffer_impl_get_bytes(Framebuffer *fb);

#endif //ALTCORE_FRAMEBUFFER_IMPL_H
