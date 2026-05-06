//
// Created by wright on 5/6/26.
//

#include "events.h"

#include <assert.h>

#include "memory.h"
#include "SDL3/SDL_init.h"

static bool g_events_initd = false;
static EventSourceFlags g_event_source_flags = 0;

typedef struct EVENTS_QUEUE_T {
    Event* data;
    i32 len;
    i32 cap;
} EventsQueue;

static EventsQueue g_events_arrays[EVENT_SOURCE_COUNT] = {};

void events_init(const EventInitInfo *info) {
    if (g_events_initd) {
        return;
    }

    bool success = SDL_InitSubSystem(SDL_INIT_EVENTS);
    assert(success);

    g_event_source_flags = info->sources;

    for (i32 source_idx = 0; source_idx < EVENT_SOURCE_COUNT; source_idx++) {
        EventSourceFlag source_flag = 1ULL << source_idx;

        EventsQueue* events_queue = &g_events_arrays[source_idx];

        if (g_event_source_flags & source_flag) {
            events_queue->data = alt_calloc(
                info->event_queue_cap,
                sizeof(*g_events_arrays[source_idx].data)
            );

            events_queue->cap = info->event_queue_cap;
        } else {
            events_queue->data = nullptr;
            events_queue->cap = 0;
        }

        events_queue->len = 0;
    }

    g_events_initd = true;
}

void events_poll() {
}

void events_uninit() {
    if (!g_events_initd) {
        return;
    }

    for (i32 source_idx = 0; source_idx < EVENT_SOURCE_COUNT; source_idx++) {
        EventsQueue* events_array = &g_events_arrays[source_idx];
        if (events_array->data) {
            alt_free(events_array->data);
        }

        events_array->data = nullptr;
        events_array->len = 0;
        events_array->cap = 0;
    }

    g_event_source_flags = 0;

    g_events_initd = false;
}
