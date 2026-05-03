//
// Created by wright on 5/2/26.
//

#include "window.h"

#include <assert.h>
#include <threads.h>
#include <stdatomic.h>

#include "SDL3/SDL_init.h"
#include <SDL3/SDL_video.h>

#include "memory.h"

constexpr i32 kMaxSwapchainBuffers = 3;

typedef enum SWAPCHAIN_BUFFER_STATE_E {
#ifndef X_SWAPCHAIN_BUFFER_STATES
#define X_SWAPCHAIN_BUFFER_STATES \
    X(FREE) \
    X(READING) \
    X(WRITING) \
    X(SUBMIT) \
    X(COUNT)
#endif
#ifndef X
#define X(state) \
    SWAPCHAIN_BUFFER_STATE_##state,
#endif
    X_SWAPCHAIN_BUFFER_STATES
#undef X
} SwapchainBufferState;

struct SWAPCHAIN_BUFFER_T {
    SDL_Surface *surface;
    SwapchainBufferState state;
    mtx_t lock;
};

struct WINDOW_HANDLE_T {
    SDL_Window *window;
    iVec2 window_size;
    SDL_Surface *window_surface;

    struct {
        SwapchainBuffer bufs[kMaxSwapchainBuffers];
        i32 count;
    } swapchain;
};

static bool g_window_system_initd = false;
static i32 g_num_windows = 0;

typedef struct {
    WindowHandle *window_handle;
    SwapchainBuffer* swapchain_buf;
} SwapchainPresentTaskArg;

static void swapchain_present_task(void* arg) {
    SwapchainPresentTaskArg *task_arg = arg;
    SwapchainBuffer *swapchain_buf = task_arg->swapchain_buf;

    bool swapchain_buf_submitted = true;

    do {
        mtx_lock(&swapchain_buf->lock);
        swapchain_buf_submitted = (swapchain_buf->state == SWAPCHAIN_BUFFER_STATE_SUBMIT);
        mtx_unlock(&swapchain_buf->lock);
    } while (!swapchain_buf_submitted);

    mtx_lock(&swapchain_buf->lock);
    swapchain_buf->state = SWAPCHAIN_BUFFER_STATE_READING;
    mtx_unlock(&swapchain_buf->lock);

    bool success = SDL_BlitSurface(
        swapchain_buf->surface,
        nullptr,
        task_arg->window_handle->window_surface,
        nullptr
    );
    assert(success);

    mtx_lock(&swapchain_buf->lock);
    swapchain_buf->state = SWAPCHAIN_BUFFER_STATE_FREE;
    mtx_unlock(&swapchain_buf->lock);
}

void window_get_display_infos(DisplayInfos *out) {
    if (!g_window_system_initd) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
        g_window_system_initd = true;
    }

    int num_displays;
    SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);
    assert(displays);

    for (i32 display_idx = 0; display_idx < num_displays; display_idx++) {
        DisplayInfo info = {};

        SDL_DisplayID display_id = displays[display_idx];

        SDL_Rect display_rect;
        bool success = SDL_GetDisplayUsableBounds(display_id, &display_rect);
        assert(success);

        info.windowed_size.x = display_rect.w;
        info.windowed_size.y = display_rect.h;

        success = SDL_GetDisplayBounds(display_id, &display_rect);
        assert(success);

        info.fullscreen_size.x = display_rect.w;
        info.fullscreen_size.y = display_rect.h;

        ARRAY_PUSH(out, &info);
    }

    SDL_free(displays);
}

WindowHandle *window_create(const WindowCreateInfo *info) {
    WindowHandle *handle = alt_malloc(sizeof(WindowHandle));
    *handle = (WindowHandle){};

    if (!g_window_system_initd) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
        g_window_system_initd = true;
    }

    int num_displays;
    SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);
    assert(displays);
    assert(info->display_idx < num_displays);

    SDL_DisplayID selected_display = displays[info->display_idx];

    SDL_PropertiesID props = SDL_CreateProperties();

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, info->title);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, info->size.x);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, info->size.y);

    bool disable_vsync = false;

    for (i32 flag_idx = 0; flag_idx < WINDOW_FLAG_OPTION_COUNT; flag_idx++) {
        WindowFlag flag = 1ULL << flag_idx;

        if (info->flags & flag) {
            switch (flag) {
                case WINDOW_FLAG_RESIZABLE: {
                    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
                    break;
                }
                case WINDOW_FLAG_DISABLE_VSYNC: {
                    disable_vsync = true;
                    break;
                }
                default:
                    break;
            }
        }
    }

    switch (info->mode) {
        case WINDOW_MODE_BORDERLESS: {
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
        }
        case WINDOW_MODE_WINDOWED: {
            SDL_Rect display_bounds;
            SDL_GetDisplayBounds(selected_display, &display_bounds);

            SDL_SetNumberProperty(
                props,
                SDL_PROP_WINDOW_CREATE_X_NUMBER,
                display_bounds.x + info->pos.x
            );
            SDL_SetNumberProperty(
                props,
                SDL_PROP_WINDOW_CREATE_Y_NUMBER,
                display_bounds.y + info->pos.y
            );
            break;
        }
        case WINDOW_MODE_FULLSCREEN: {
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
            SDL_SetNumberProperty(
                props,
                SDL_PROP_WINDOW_CREATE_X_NUMBER,
                SDL_WINDOWPOS_CENTERED_DISPLAY(selected_display)
            );
            SDL_SetNumberProperty(
                props,
                SDL_PROP_WINDOW_CREATE_Y_NUMBER,
                SDL_WINDOWPOS_CENTERED_DISPLAY(selected_display)
            );
            break;
        }
        default:
            break;
    }

    handle->window = SDL_CreateWindowWithProperties(props);
    assert(handle->window);

    SDL_DestroyProperties(props);
    SDL_free(displays);

    g_num_windows++;

    if (disable_vsync) {
        SDL_SetWindowSurfaceVSync(handle->window, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
    }

    bool success = SDL_GetWindowSize(handle->window, &handle->window_size.x, &handle->window_size.y);
    assert(success);

    handle->window_surface = SDL_GetWindowSurface(handle->window);

    switch (info->swapchain_mode) {
        case SWAPCHAIN_MODE_DOUBLE_BUFFERED: {
            handle->swapchain.count = 2;
            break;
        }
        case SWAPCHAIN_MODE_TRIPLE_BUFFERED: {
            handle->swapchain.count = 3;
            break;
        }
        default:
            assert(0 && "Unhandled swapchain mode");
            break;
    }

    for (i32 swapchain_idx = 0; swapchain_idx < handle->swapchain.count; swapchain_idx++) {
        handle->swapchain.bufs[swapchain_idx].surface = SDL_CreateSurface(
            handle->window_surface->w,
            handle->window_surface->h,
            handle->window_surface->format
        );

        assert(handle->swapchain.bufs[swapchain_idx].surface);

        int err = mtx_init(&handle->swapchain.bufs[swapchain_idx].lock, mtx_plain);
        assert(!err);

        handle->swapchain.bufs[swapchain_idx].state = SWAPCHAIN_BUFFER_STATE_FREE;
    }

    return handle;
}

void window_destroy(WindowHandle *handle) {
    assert(g_window_system_initd);

    for (i32 swapchain_idx = 0; swapchain_idx < handle->swapchain.count; swapchain_idx++) {
        SwapchainBuffer *buf = &handle->swapchain.bufs[swapchain_idx];
        SDL_DestroySurface(buf->surface);
        mtx_destroy(&buf->lock);
    }

    handle->swapchain.count = 0;

    SDL_DestroyWindow(handle->window);

    alt_free(handle);

    g_num_windows--;

    if (!g_num_windows) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        g_window_system_initd = false;
    }
}

i32 window_get_swapchain_bufs_count(const WindowHandle *handle) {
    return handle->swapchain.count;
}

SwapchainBuffer *window_get_free_swapchain_buf(WindowHandle *handle) {
    SwapchainBuffer *buf = nullptr;

    i32 current_buf_idx = 0;

    while (!buf) {
        SwapchainBuffer *current_buf = &handle->swapchain.bufs[current_buf_idx];

        mtx_lock(&current_buf->lock);

        if (current_buf->state == SWAPCHAIN_BUFFER_STATE_FREE) {
            buf = current_buf;
        }

        mtx_unlock(&current_buf->lock);

        current_buf_idx++;
        current_buf_idx = current_buf_idx % handle->swapchain.count;
    }

    return buf;
}

void window_present_swapchain_buf(WindowHandle *handle, const SwapchainBuffer *buf) {
    i64 buf_idx = buf - handle->swapchain.bufs;
    assert(buf_idx >= 0 && buf_idx < handle->swapchain.count);

}
