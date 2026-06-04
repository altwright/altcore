//
// Created by wright on 5/9/26.
//

#include "present.h"

#include "../../memory.h"

void soft_cmd_present(
    Framebuffer* fb,
    WindowHandle* window
) {
    window_present_framebuffer(window, fb);
}
