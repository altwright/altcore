//
// Created by wright on 5/25/26.
//

#include "scissor.h"

#include "../framebuffer.impl.h"

void soft_cmd_scissor(RenderCmdScissor *data) {
    framebuffer_impl_set_scissor(data->framebuffer, data->region);
}
