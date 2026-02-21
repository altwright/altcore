//
// Created by wright on 2/18/26.
//

#include "defer.h"

#include <assert.h>
#include <threads.h>

static bool g_end_of_defer = false;;
static mtx_t g_lock = {};
static bool g_initialized = false;

void defer_init() {
    if (!g_initialized) {
        int res = mtx_init(&g_lock, mtx_plain);
        assert(res == thrd_success);
        g_initialized = true;
    }
}

void defer_uninit() {
    if (g_initialized) {
        mtx_destroy(&g_lock);
        g_lock = (mtx_t){};
        g_initialized = false;
    }
}

void defer_lock_false() {
    assert(g_initialized);

    int res = mtx_lock(&g_lock);
    assert(res == thrd_success);

    g_end_of_defer = false;
}

void defer_lock_true() {
    assert(g_initialized);

    int res = mtx_lock(&g_lock);
    assert(res == thrd_success);

    g_end_of_defer = true;
}

bool defer_is_false_unlock() {
    assert(g_initialized);

    bool is_false = !g_end_of_defer;

    int res = mtx_unlock(&g_lock);
    assert(res == thrd_success);

    return is_false;
}
