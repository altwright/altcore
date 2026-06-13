//
// Created by wright on 5/6/26.
//

#include "events.h"

#include <assert.h>

#include "memory.h"
#include "SDL3/SDL_init.h"
#include "draw/window.h"
#include "draw/window_impl.h"

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
    window_impl_update_surfaces();

    g_events_q.count = g_events_q.head_idx = 0;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        Event e = {};
        bool handled = true;
        switch (event.type) {
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                if (!(g_event_source_flags & EVENT_SOURCE_FLAG_WINDOW)) {
                    handled = false;
                    break;
                }

                WindowEvent we = {};

                switch (event.type) {
                    case SDL_EVENT_WINDOW_RESIZED: {
                        we.type = WINDOW_EVENT_RESIZE;
                        we.data.resize.new_size = (i32x2){
                            .width = event.window.data1,
                            .height = event.window.data2,
                        };
                        we.data.resize.window = window_impl_get_handle_from_id(event.window.windowID);
                        break;
                    }
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                        we.type = WINDOW_EVENT_CLOSE;
                        we.data.close.window = window_impl_get_handle_from_id(event.window.windowID);
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
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                if (!(g_event_source_flags & EVENT_SOURCE_FLAG_KEYBOARD)) {
                    handled = false;
                    break;
                }

                e.source = EVENT_SOURCE_KEYBOARD;

                switch (event.type) {
                    case SDL_EVENT_KEY_DOWN: {
                        e.data.keyboard.type = KEYBOARD_EVENT_KEY_PRESS;
                        break;
                    }
                    case SDL_EVENT_KEY_UP: {
                        e.data.keyboard.type = KEYBOARD_EVENT_KEY_RELEASE;
                        break;
                    }
                    default:
                        handled = false;
                        break;
                }

                KeyboardKey *key = &e.data.keyboard.key;

                switch (event.key.key) {
                    case SDLK_ESCAPE: *key = KEYBOARD_KEY_ESCAPE;
                        break;
                    case SDLK_F1: *key = KEYBOARD_KEY_F1;
                        break;
                    case SDLK_F2: *key = KEYBOARD_KEY_F2;
                        break;
                    case SDLK_F3: *key = KEYBOARD_KEY_F3;
                        break;
                    case SDLK_F4: *key = KEYBOARD_KEY_F4;
                        break;
                    case SDLK_F5: *key = KEYBOARD_KEY_F5;
                        break;
                    case SDLK_F6: *key = KEYBOARD_KEY_F6;
                        break;
                    case SDLK_F7: *key = KEYBOARD_KEY_F7;
                        break;
                    case SDLK_F8: *key = KEYBOARD_KEY_F8;
                        break;
                    case SDLK_F9: *key = KEYBOARD_KEY_F9;
                        break;
                    case SDLK_F10: *key = KEYBOARD_KEY_F10;
                        break;
                    case SDLK_F11: *key = KEYBOARD_KEY_F11;
                        break;
                    case SDLK_F12: *key = KEYBOARD_KEY_F12;
                        break;
                    case SDLK_GRAVE: *key = KEYBOARD_KEY_BACKTICK;
                        break;
                    case SDLK_1: *key = KEYBOARD_KEY_NUM_1;
                        break;
                    case SDLK_2: *key = KEYBOARD_KEY_NUM_2;
                        break;
                    case SDLK_3: *key = KEYBOARD_KEY_NUM_3;
                        break;
                    case SDLK_4: *key = KEYBOARD_KEY_NUM_4;
                        break;
                    case SDLK_5: *key = KEYBOARD_KEY_NUM_5;
                        break;
                    case SDLK_6: *key = KEYBOARD_KEY_NUM_6;
                        break;
                    case SDLK_7: *key = KEYBOARD_KEY_NUM_7;
                        break;
                    case SDLK_8: *key = KEYBOARD_KEY_NUM_8;
                        break;
                    case SDLK_9: *key = KEYBOARD_KEY_NUM_9;
                        break;
                    case SDLK_0: *key = KEYBOARD_KEY_NUM_0;
                        break;
                    case SDLK_MINUS: *key = KEYBOARD_KEY_MINUS;
                        break;
                    case SDLK_EQUALS: *key = KEYBOARD_KEY_EQUALS;
                        break;
                    case SDLK_BACKSPACE: *key = KEYBOARD_KEY_BACKSPACE;
                        break;
                    case SDLK_TAB: *key = KEYBOARD_KEY_TAB;
                        break;
                    case SDLK_Q: *key = KEYBOARD_KEY_Q;
                        break;
                    case SDLK_W: *key = KEYBOARD_KEY_W;
                        break;
                    case SDLK_E: *key = KEYBOARD_KEY_E;
                        break;
                    case SDLK_R: *key = KEYBOARD_KEY_R;
                        break;
                    case SDLK_T: *key = KEYBOARD_KEY_T;
                        break;
                    case SDLK_Y: *key = KEYBOARD_KEY_Y;
                        break;
                    case SDLK_U: *key = KEYBOARD_KEY_U;
                        break;
                    case SDLK_I: *key = KEYBOARD_KEY_I;
                        break;
                    case SDLK_O: *key = KEYBOARD_KEY_O;
                        break;
                    case SDLK_P: *key = KEYBOARD_KEY_P;
                        break;
                    case SDLK_LEFTBRACKET: *key = KEYBOARD_KEY_LEFT_BRACKET;
                        break;
                    case SDLK_RIGHTBRACKET: *key = KEYBOARD_KEY_RIGHT_BRACKET;
                        break;
                    case SDLK_BACKSLASH: *key = KEYBOARD_KEY_BACKSLASH;
                        break;
                    case SDLK_CAPSLOCK: *key = KEYBOARD_KEY_CAPS_LOCK;
                        break;
                    case SDLK_A: *key = KEYBOARD_KEY_A;
                        break;
                    case SDLK_S: *key = KEYBOARD_KEY_S;
                        break;
                    case SDLK_D: *key = KEYBOARD_KEY_D;
                        break;
                    case SDLK_F: *key = KEYBOARD_KEY_F;
                        break;
                    case SDLK_G: *key = KEYBOARD_KEY_G;
                        break;
                    case SDLK_H: *key = KEYBOARD_KEY_H;
                        break;
                    case SDLK_J: *key = KEYBOARD_KEY_J;
                        break;
                    case SDLK_K: *key = KEYBOARD_KEY_K;
                        break;
                    case SDLK_L: *key = KEYBOARD_KEY_L;
                        break;
                    case SDLK_SEMICOLON: *key = KEYBOARD_KEY_SEMICOLON;
                        break;
                    case SDLK_APOSTROPHE: *key = KEYBOARD_KEY_APOSTROPHE;
                        break;
                    case SDLK_RETURN: *key = KEYBOARD_KEY_ENTER;
                        break;
                    case SDLK_LSHIFT: *key = KEYBOARD_KEY_LEFT_SHIFT;
                        break;
                    case SDLK_Z: *key = KEYBOARD_KEY_Z;
                        break;
                    case SDLK_X: *key = KEYBOARD_KEY_X_KEY;
                        break;
                    case SDLK_C: *key = KEYBOARD_KEY_C;
                        break;
                    case SDLK_V: *key = KEYBOARD_KEY_V;
                        break;
                    case SDLK_B: *key = KEYBOARD_KEY_B;
                        break;
                    case SDLK_N: *key = KEYBOARD_KEY_N;
                        break;
                    case SDLK_M: *key = KEYBOARD_KEY_M;
                        break;
                    case SDLK_COMMA: *key = KEYBOARD_KEY_COMMA;
                        break;
                    case SDLK_PERIOD: *key = KEYBOARD_KEY_PERIOD;
                        break;
                    case SDLK_SLASH: *key = KEYBOARD_KEY_SLASH;
                        break;
                    case SDLK_RSHIFT: *key = KEYBOARD_KEY_RIGHT_SHIFT;
                        break;
                    case SDLK_LCTRL: *key = KEYBOARD_KEY_LEFT_CTRL;
                        break;
                    case SDLK_LGUI: *key = KEYBOARD_KEY_LEFT_CMD;
                        break;
                    case SDLK_LALT: *key = KEYBOARD_KEY_LEFT_ALT;
                        break;
                    case SDLK_SPACE: *key = KEYBOARD_KEY_SPACE;
                        break;
                    case SDLK_RALT: *key = KEYBOARD_KEY_RIGHT_ALT;
                        break;
                    case SDLK_RGUI: *key = KEYBOARD_KEY_RIGHT_CMD;
                        break;
                    case SDLK_MENU: *key = KEYBOARD_KEY_MENU;
                        break;
                    case SDLK_RCTRL: *key = KEYBOARD_KEY_RIGHT_CTRL;
                        break;
                    case SDLK_PRINTSCREEN: *key = KEYBOARD_KEY_PRINT_SCREEN;
                        break;
                    case SDLK_SCROLLLOCK: *key = KEYBOARD_KEY_SCROLL_LOCK;
                        break;
                    case SDLK_PAUSE: *key = KEYBOARD_KEY_PAUSE;
                        break;
                    case SDLK_INSERT: *key = KEYBOARD_KEY_INSERT;
                        break;
                    case SDLK_HOME: *key = KEYBOARD_KEY_HOME;
                        break;
                    case SDLK_PAGEUP: *key = KEYBOARD_KEY_PAGE_UP;
                        break;
                    case SDLK_DELETE: *key = KEYBOARD_KEY_DELETE_KEY;
                        break;
                    case SDLK_END: *key = KEYBOARD_KEY_END;
                        break;
                    case SDLK_PAGEDOWN: *key = KEYBOARD_KEY_PAGE_DOWN;
                        break;
                    case SDLK_UP: *key = KEYBOARD_KEY_ARROW_UP;
                        break;
                    case SDLK_LEFT: *key = KEYBOARD_KEY_ARROW_LEFT;
                        break;
                    case SDLK_DOWN: *key = KEYBOARD_KEY_ARROW_DOWN;
                        break;
                    case SDLK_RIGHT: *key = KEYBOARD_KEY_ARROW_RIGHT;
                        break;
                    default:
                        handled = false;
                        break;
                }

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

    g_events_q.head_idx += (i32) fetch;
    g_events_q.count -= (i32) fetch;

    return g_events_q.count;
}

void events_deinit() {
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
