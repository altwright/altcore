//
// Created by wright on 5/3/26.
//

#include "renderer.h"

#include <threads.h>

static bool g_render_thread_initd = false;
static thrd_t g_render_thread;

static int render_thread_fn(void* arg) {
    return 0;
}
