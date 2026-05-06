//
// Created by wright on 5/6/26.
//

#ifndef ALTCORE_EVENTS_H
#define ALTCORE_EVENTS_H

#include "draw/window.h"

typedef enum EVENT_SOURCE_FLAG_OPTION_E {
#ifndef X_EVENT_SOURCE_FLAG_OPTIONS
#define X_EVENT_SOURCE_FLAG_OPTIONS \
    X(KEYBOARD) \
    X(MOUSE) \
    X(WINDOW) \
    X(TOUCH) \
    X(GAMEPAD) \
    X(COUNT)
#endif
#ifndef X
#define X(option) \
    EVENT_SOURCE_FLAG_OPTION_##option,
#endif
    X_EVENT_SOURCE_FLAG_OPTIONS
#undef X
} EventSourceFlagOption;

typedef enum EVENT_SOURCE_FLAG_E : u64 {
#ifndef X
#define X(option) \
    EVENT_SOURCE_FLAG_##option = 1ULL << EVENT_SOURCE_FLAG_OPTION_##option,
#endif
    X_EVENT_SOURCE_FLAG_OPTIONS
#undef X
} EventSourceFlag;

typedef u64 EventSourceFlags;

void events_init(EventSourceFlags event_sources);

void events_poll();

void events_uninit();

#endif //ALTCORE_EVENTS_H
