//
// Created by wright on 5/6/26.
//

#ifndef ALTCORE_EVENTS_H
#define ALTCORE_EVENTS_H

#include "draw/window.h"

typedef enum EVENT_SOURCE_E {
#ifndef X_EVENT_SOURCES
#define X_EVENT_SOURCES \
    X(KEYBOARD) \
    X(MOUSE) \
    X(WINDOW) \
    X(TOUCH) \
    X(GAMEPAD) \
    X(COUNT)
#endif
#ifndef X
#define X(source) \
    EVENT_SOURCE_##source,
#endif
    X_EVENT_SOURCES
#undef X
} EventSource;

typedef enum EVENT_SOURCE_FLAG_E : u64 {
#ifndef X
#define X(source) \
    EVENT_SOURCE_FLAG_##source = 1ULL << EVENT_SOURCE_##source,
#endif
    X_EVENT_SOURCES
#undef X
} EventSourceFlag;

typedef u64 EventSourceFlags;

typedef struct EVENT_INIT_INFO_T {
    EventSourceFlags sources;
    i32 event_queue_cap;
} EventInitInfo;

typedef enum WINDOW_EVENT_TYPE_E {
#ifndef X_WINDOW_EVENT_TYPES
#define X_WINDOW_EVENT_TYPES \
    X(CLOSE) \
    X(RESIZE) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    WINDOW_EVENT_##type,
#endif
    X_WINDOW_EVENT_TYPES
#undef X
} WindowEventType;

typedef struct WINDOW_EVENT_T {
    WindowEventType type;
} WindowEvent;

typedef struct KEYBOARD_EVENT_TYPE_E {
#ifndef X_KEYBOARD_EVENT_TYPES
#define X_KEYBOARD_EVENT_TYPES \
    X(KEY_PRESS) \
    X(KEY_RELEASE) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    KEYBOARD_EVENT_##type,
#endif
#undef X
} KeyboardEventType;

typedef struct KEYBOARD_EVENT_T {
    KeyboardEventType type;
} KeyboardEvent;

typedef struct EVENT_T {
    EventSourceFlag source;
    union {
        WindowEvent window;
        KeyboardEvent keyboard;
    } data;
} Event;

typedef struct EVENTS_T {
    ARRAY_FIELDS(Event)
} Events;

void events_init(const EventInitInfo* info);

void events_poll();

void events_get(EventSource source, Events* events);

void events_uninit();

#endif //ALTCORE_EVENTS_H
