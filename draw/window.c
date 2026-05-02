//
// Created by wright on 5/2/26.
//

#include "window.h"

#include <assert.h>

#include "SDL3/SDL_init.h"
#include <SDL3/SDL_video.h>

#include "memory.h"

struct WINDOW_HANDLE_T {
    SDL_Window *sdl_window;
};

static bool g_sdl_video_initd = false;
static i32 g_num_windows = 0;

WindowHandle *window_create(const WindowCreateInfo *info) {
    WindowHandle *handle = alt_malloc(sizeof(WindowHandle));
    *handle = (WindowHandle){};

    if (!g_sdl_video_initd) {
        SDL_Init(SDL_INIT_VIDEO);
        g_sdl_video_initd = true;
    }

    SDL_WindowFlags sdl_flags = {};

    for (i32 flag_idx = 0; flag_idx < WINDOW_FLAG_OPTION_COUNT; flag_idx++) {
        WindowFlag flag = 1ULL << flag_idx;

        if (info->flags & flag) {
            switch (flag) {
                case WINDOW_FLAG_RESIZABLE: {
                    sdl_flags |= SDL_WINDOW_RESIZABLE;
                    break;
                }
                default:
                    break;
            }
        }
    }

    switch (info->mode) {
        case WINDOW_MODE_FULLSCREEN: {
            sdl_flags |= SDL_WINDOW_FULLSCREEN;
            break;
        }
        case WINDOW_MODE_BORDERLESS: {
            sdl_flags |= SDL_WINDOW_BORDERLESS;
            break;
        }
        default:
            break;
    }

    handle->sdl_window = SDL_CreateWindow(info->title, info->size.x, info->size.y, sdl_flags);
    assert(handle->sdl_window);

    g_num_windows++;

    return handle;
}

void window_destroy(WindowHandle* handle) {
    assert(g_sdl_video_initd);

    SDL_DestroyWindow(handle->sdl_window);

    alt_free(handle);

    g_num_windows--;

    if (!g_num_windows) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        g_sdl_video_initd = false;
    }
}
