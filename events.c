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
    Event *data;
    i32 head_idx;
    i32 count;
    i32 cap;
} EventsQueue;

static EventsQueue g_events_q = {};

void events_init(const EventInitInfo *info) {
    if (g_events_initd) {
        return;
    }

    bool success = SDL_InitSubSystem(SDL_INIT_EVENTS);
    assert(success);

    g_event_source_flags = info->sources;

    g_events_q.data = alt_calloc(info->event_q_max_cap, sizeof(*g_events_q.data));
    assert(g_events_q.data);
    g_events_q.head_idx = 0;
    g_events_q.count = 0;
    g_events_q.cap = info->event_q_max_cap;

    g_events_initd = true;
}

void events_poll() {
    g_events_q.count = g_events_q.head_idx = 0;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        Event e = {};
        bool handled = true;
        switch (event.type) {
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                WindowEvent we = {};

                switch (event.type) {
                    case SDL_EVENT_WINDOW_RESIZED: {
                        we.type = WINDOW_EVENT_RESIZE;
                        we.data.resize.new_size = (iVec2){
                            event.window.data1,
                            event.window.data2,
                        };
                        we.data.resize.window_id = event.window.windowID;
                        break;
                    }
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                        we.type = WINDOW_EVENT_CLOSE;
                        we.data.close.window_id = event.window.windowID;
                        break;
                    }
                    default:
                        handled = false;
                        break;
                }

                e.source = EVENT_SOURCE_WINDOW;
                e.data.window = we;
                break;
            }
            default:
                handled = false;
                break;
        }

        if (handled) {
            g_events_q.data[g_events_q.count++] = e;
        }

        if (g_events_q.count >= g_events_q.cap) {
            break;
        }
    }
}

i32 events_get(Events *array) {
    if (!array) {
        return g_events_q.count;
    }

    i64 array_space = array->cap - array->len;

    i64 fetch = array_space < g_events_q.count ? array_space : g_events_q.count;

    for (i32 event_offset = 0; event_offset < fetch; event_offset++) {
        ARRAY_PUSH(array, &g_events_q.data[g_events_q.head_idx + event_offset]);
    }

    g_events_q.head_idx += (i32)fetch;
    g_events_q.count -= (i32)fetch;

    return g_events_q.count;
}

void events_uninit() {
    if (!g_events_initd) {
        return;
    }

    alt_free(g_events_q.data);
    g_events_q.head_idx = 0;
    g_events_q.count = 0;
    g_events_q.cap = 0;

    g_event_source_flags = 0;

    g_events_initd = false;
}
