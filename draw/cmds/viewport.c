//
// Created by wright on 10/23/26.
//

#include "viewport.h"

#include "../framebuffer.impl.h"

void soft_cmd_viewport(RenderCmdViewport *data) {
    framebuffer_impl_set_viewport(data->framebuffer, data->region);
}
