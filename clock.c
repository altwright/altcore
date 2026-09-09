//
// Created by wright on 9/9/26.
//

#include "clock.h"

#include <time.h>

#include "debug.h"

static bool g_clock_initialized = false;
static u64 g_clock_start_time_ns = 0;

static constexpr u64 kMaxClockResolutionNs = 1000000;
static constexpr u64 kOneSecNs = 1000000000;

void clock_init() {
    if (!g_clock_initialized) {
        g_clock_initialized = true;

        struct timespec ts;
        int err = clock_getres(CLOCK_MONOTONIC, &ts);
        if (err) {
            crash_msg("clock_getres failed: %d\n", err);
        }

        if (ts.tv_nsec >= kMaxClockResolutionNs) {
            crash_msg("Clock resolution exceeds %u\n", kMaxClockResolutionNs);
        }

        err = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (err) {
            crash_msg("clock_gettime error: %d\n", err);
        }

        g_clock_start_time_ns = ts.tv_sec * kOneSecNs + ts.tv_nsec;
    }
}

void clock_deinit() {
    if (g_clock_initialized) {
        g_clock_start_time_ns = 0;
        g_clock_initialized = false;
    }
}

u64 clock_get_elapsed_ns() {
    if (!g_clock_initialized) {
        crash_msg("Cannot get time: clock not initialized\n");
    }

    struct timespec ts;
    int err = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (err) {
        crash_msg("clock_gettime error: %d\n", err);
    }

    u64 current_ns = ts.tv_sec * kOneSecNs + ts.tv_nsec;
    if (current_ns < g_clock_start_time_ns) {
        crash_msg("Current time %llu precedes start time %llu\n", current_ns, g_clock_start_time_ns);
    }

    return current_ns - g_clock_start_time_ns;
}
