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

typedef EventSourceFlag EventSourceFlags;

typedef struct EVENT_INIT_INFO_T {
    EventSourceFlags sources;
    i32 event_q_max_cap;
} EventInitInfo;

typedef enum SYSTEM_EVENT_TYPE_E {
#ifndef X_SYSTEM_EVENT_TYPES
#define X_SYSTEM_EVENT_TYPES \
    X(FOCUS) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    SYSTEM_EVENT_TYPE_##type,
#endif
    X_SYSTEM_EVENT_TYPES
#undef X
} SystemEventType;

typedef struct SYSTEM_EVENT_T {
    SystemEventType type;
} SystemEvent;

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

typedef struct WINDOW_EVENT_RESIZE_T {
    i32x2 new_size;
    WindowHandle *window;
} WindowEventResize;

typedef struct WINDOW_EVENT_CLOSE_T {
    WindowHandle *window;
} WindowEventClose;

typedef struct WINDOW_EVENT_T {
    WindowEventType type;

    union {
        WindowEventResize resize;
        WindowEventClose close;
    } data;
} WindowEvent;

typedef enum KEYBOARD_EVENT_TYPE_E {
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
    X_KEYBOARD_EVENT_TYPES
#undef X
} KeyboardEventType;

typedef enum KEYBOARD_KEY_E {
#ifndef X_KEYBOARD_KEYS
#define X_KEYBOARD_KEYS \
    /* Function keys */ \
    X(ESCAPE)        \
    X(F1)            \
    X(F2)            \
    X(F3)            \
    X(F4)            \
    X(F5)            \
    X(F6)            \
    X(F7)            \
    X(F8)            \
    X(F9)            \
    X(F10)           \
    X(F11)           \
    X(F12)           \
    /* Number row */ \
    X(BACKTICK)      \
    X(NUM_1)         \
    X(NUM_2)         \
    X(NUM_3)         \
    X(NUM_4)         \
    X(NUM_5)         \
    X(NUM_6)         \
    X(NUM_7)         \
    X(NUM_8)         \
    X(NUM_9)         \
    X(NUM_0)         \
    X(MINUS)         \
    X(EQUALS)        \
    X(BACKSPACE)     \
    /* Top alphabetic row */ \
    X(TAB)           \
    X(Q)             \
    X(W)             \
    X(E)             \
    X(R)             \
    X(T)             \
    X(Y)             \
    X(U)             \
    X(I)             \
    X(O)             \
    X(P)             \
    X(LEFT_BRACKET)  \
    X(RIGHT_BRACKET) \
    X(BACKSLASH)     \
    /* Home row */ \
    X(CAPS_LOCK)     \
    X(A)             \
    X(S)             \
    X(D)             \
    X(F)             \
    X(G)             \
    X(H)             \
    X(J)             \
    X(K)             \
    X(L)             \
    X(SEMICOLON)     \
    X(APOSTROPHE)    \
    X(ENTER)         \
    /* Bottom alphabetic row */ \
    X(LEFT_SHIFT)    \
    X(Z)             \
    X(X_KEY)         \
    X(C)             \
    X(V)             \
    X(B)             \
    X(N)             \
    X(M)             \
    X(COMMA)         \
    X(PERIOD)        \
    X(SLASH)         \
    X(RIGHT_SHIFT)   \
    /* Bottom row */ \
    X(LEFT_CTRL)     \
    X(LEFT_CMD)      \
    X(LEFT_ALT)      \
    X(SPACE)         \
    X(RIGHT_ALT)     \
    X(RIGHT_CMD)     \
    X(MENU)          \
    X(RIGHT_CTRL)    \
    /* Navigation cluster */ \
    X(PRINT_SCREEN)  \
    X(SCROLL_LOCK)   \
    X(PAUSE)         \
    X(INSERT)        \
    X(HOME)          \
    X(PAGE_UP)       \
    X(DELETE_KEY)    \
    X(END)           \
    X(PAGE_DOWN)     \
    /* Arrow keys */ \
    X(ARROW_UP)      \
    X(ARROW_LEFT)    \
    X(ARROW_DOWN)    \
    X(ARROW_RIGHT) \
    X(COUNT)
#endif
#ifndef X
#define X(key) \
    KEYBOARD_KEY_##key,
#endif
    X_KEYBOARD_KEYS
#undef X
} KeyboardKey;

typedef struct KEYBOARD_EVENT_T {
    KeyboardEventType type;
    KeyboardKey key;
} KeyboardEvent;

typedef enum MOUSE_EVENT_TYPE_E {
#ifndef X_MOUSE_EVENT_TYPES
#define X_MOUSE_EVENT_TYPES \
    X(MOVE) \
    X(CLICK) \
    X(SCROLL) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    MOUSE_EVENT_TYPE_##type,
#endif
    X_MOUSE_EVENT_TYPES
#undef X
} MouseEventType;

typedef struct MOUSE_EVENT_T {
    MouseEventType type;
} MouseEvent;

typedef struct EVENT_T {
    EventSource source;

    union {
        SystemEvent system;
        WindowEvent window;
        KeyboardEvent keyboard;
        MouseEvent mouse;
    } data;
} Event;

typedef struct EVENTS_T {
    ARRAY_FIELDS(Event)
} Events;

void events_init(const EventInitInfo *info);

void events_poll();

// Returns the number of events remaining in the queue
// since last poll, after they have been copied into array
i32 events_get(Events *array);

void events_uninit();

#endif //ALTCORE_EVENTS_H
