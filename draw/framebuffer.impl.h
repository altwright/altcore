//
// Created by wright on 5/16/26.
//

#ifndef ALTCORE_FRAMEBUFFER_IMPL_H
#define ALTCORE_FRAMEBUFFER_IMPL_H

#include "framebuffer.h"

void framebuffer_impl_get_bytes(Framebuffer *fb, u8** out_bytes, i64* out_pitch);

void framebuffer_impl_set_scissor(Framebuffer *fb, f32x4 region);

#endif //ALTCORE_FRAMEBUFFER_IMPL_H
