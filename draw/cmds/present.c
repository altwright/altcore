//
// Created by wright on 5/9/26.
//

#include "present.h"

#include "../../memory.h"

typedef struct SOFT_RENDERER_PRESENT_TASK_ARG_T {
    WindowHandle* window;
    SwapchainBuffer* swapchain_buf;
} SoftRendererPresentTaskArg;

static void soft_renderer_present_task(void* arg) {
    SoftRendererPresentTaskArg* task_arg = arg;

    window_present_swapchain_buf(task_arg->window, task_arg->swapchain_buf);

    alt_free(task_arg);
}

void soft_renderer_present(WindowHandle* window, SwapchainBuffer* swapchain_buf, Worker* worker) {
    SoftRendererPresentTaskArg *task_arg = alt_malloc(sizeof(*task_arg));
    *task_arg = (SoftRendererPresentTaskArg){
        .window = window,
        .swapchain_buf = swapchain_buf,
    };

    Task task = {
        .fn_ptr = soft_renderer_present_task,
        .arg = task_arg,
    };

    worker_push_task(worker, &task);
}
