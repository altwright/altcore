//
// Created by wright on 5/9/26.
//

#ifndef ALTCORE_PRESENT_H
#define ALTCORE_PRESENT_H

#include "../window.h"
#include "../../worker.h"

void soft_renderer_present(WindowHandle* window, SwapchainBuffer* swapchain_buf, Worker* worker);

#endif //ALTCORE_PRESENT_H
