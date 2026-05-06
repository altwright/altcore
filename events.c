//
// Created by wright on 5/6/26.
//

#include "events.h"

#include <assert.h>

#include "SDL3/SDL_init.h"

static bool g_events_initd = false;
static EventSourceFlags g_event_source_flags = 0;

void events_init(EventSourceFlags event_sources) {
    if (g_events_initd) {
        return;
    }

    bool success = SDL_InitSubSystem(SDL_INIT_EVENTS);
    assert(success);

    g_event_source_flags = event_sources;

    g_events_initd = true;
}

void events_poll() {

}

void events_uninit() {
    if (!g_events_initd) {
        return;
    }

    g_event_source_flags = 0;

    g_events_initd = false;
}
