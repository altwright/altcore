//
// Created by wright on 5/2/26.
//

#include "window.h"

#include <assert.h>
#include <stdatomic.h>

#include "SDL3/SDL_init.h"
#include <SDL3/SDL_video.h>

#include "../debug.h"
#include "../memory.h"
#include "../worker.h"

constexpr i32 kMaxSwapchainBuffers = 3;

typedef enum SWAPCHAIN_BUFFER_STATE_E {
#ifndef X_SWAPCHAIN_BUFFER_STATES
#define X_SWAPCHAIN_BUFFER_STATES \
    X(FREE) \
    X(RENDERING_TO) \
    X(WAITING_TO_PRESENT) \
    X(INVALIDATED) \
    X(PRESENTING) \
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
    _Atomic SwapchainBufferState state;
};

struct WINDOW_HANDLE_T {
    SDL_Window *window;
    SDL_WindowID id;
    iVec2 window_size;
    SDL_Surface *window_surface;

    struct {
        SwapchainBuffer bufs[kMaxSwapchainBuffers];
        i32 count;
        Worker* presenter;

        i32 next_free_idx;
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

    SwapchainBufferState state = SWAPCHAIN_BUFFER_STATE_COUNT;

    while (
        state != SWAPCHAIN_BUFFER_STATE_WAITING_TO_PRESENT
        && state != SWAPCHAIN_BUFFER_STATE_INVALIDATED
    ) {
        state = swapchain_buf->state;
    }

    if (state == SWAPCHAIN_BUFFER_STATE_WAITING_TO_PRESENT) {
        atomic_store(&swapchain_buf->state, SWAPCHAIN_BUFFER_STATE_PRESENTING);

        bool success = SDL_BlitSurface(
            swapchain_buf->surface,
            nullptr,
            task_arg->window_handle->window_surface,
            nullptr
        );
        assert(success);
        SDL_UpdateWindowSurface(task_arg->window_handle->window);
    }

    atomic_store(&swapchain_buf->state, SWAPCHAIN_BUFFER_STATE_FREE);

    alt_free(task_arg);
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

    handle->id = SDL_GetWindowID(handle->window);

    if (disable_vsync) {
        SDL_SetWindowSurfaceVSync(handle->window, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
    }

    bool success = SDL_GetWindowSize(handle->window, &handle->window_size.x, &handle->window_size.y);
    assert(success);

    handle->window_surface = SDL_GetWindowSurface(handle->window);
    SDL_UpdateWindowSurface(handle->window);

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

        handle->swapchain.bufs[swapchain_idx].state = SWAPCHAIN_BUFFER_STATE_FREE;
    }

    handle->swapchain.next_free_idx = 0;

    WorkerCreateInfo presenter_info = {
        .task_q_cap = kMaxSwapchainBuffers,
    };
    handle->swapchain.presenter = worker_create(&presenter_info);

    return handle;
}

void window_destroy(WindowHandle *handle) {
    assert(g_window_system_initd);

    worker_destroy(handle->swapchain.presenter);
    handle->swapchain.presenter = nullptr;

    for (i32 swapchain_idx = 0; swapchain_idx < handle->swapchain.count; swapchain_idx++) {
        SwapchainBuffer *buf = &handle->swapchain.bufs[swapchain_idx];
        SDL_DestroySurface(buf->surface);
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
    SwapchainBuffer *buf = &handle->swapchain.bufs[handle->swapchain.next_free_idx];

    SwapchainBufferState state = SWAPCHAIN_BUFFER_STATE_COUNT;

    while (state != SWAPCHAIN_BUFFER_STATE_FREE) {
        state = atomic_load(&buf->state);
    }

    atomic_store(&buf->state, SWAPCHAIN_BUFFER_STATE_RENDERING_TO);

    i32 next_free_idx = handle->swapchain.next_free_idx;
    handle->swapchain.next_free_idx = (next_free_idx + 1) % handle->swapchain.count;

    return buf;
}

void window_present_swapchain_buf(WindowHandle *handle, SwapchainBuffer *buf) {
    i32 buf_idx = (i32)(buf - handle->swapchain.bufs);
    assert(buf_idx >= 0 && buf_idx < handle->swapchain.count);

    i32 prev_buf_ifx = (buf_idx - 1) % handle->swapchain.count;
    SwapchainBuffer* prev_buf = &handle->swapchain.bufs[prev_buf_ifx];

    if (atomic_load(&prev_buf->state) == SWAPCHAIN_BUFFER_STATE_WAITING_TO_PRESENT) {
        atomic_store(&prev_buf->state, SWAPCHAIN_BUFFER_STATE_INVALIDATED);
    }

    atomic_store(&buf->state, SWAPCHAIN_BUFFER_STATE_WAITING_TO_PRESENT);

    SwapchainPresentTaskArg* task_arg = alt_malloc(sizeof(SwapchainPresentTaskArg));

    *task_arg = (SwapchainPresentTaskArg) {
        .window_handle = handle,
        .swapchain_buf = buf,
    };

    Task task = {
        .fn_ptr = swapchain_present_task,
        .arg = task_arg,
    };

    worker_push_task(handle->swapchain.presenter, &task);
}

SwapchainBufferData swapchain_open(SwapchainBuffer* buf) {
    SDL_LockSurface(buf->surface);

    SwapchainBufferData data = {
        .pixels = buf->surface->pixels,
        .width = buf->surface->w,
        .height = buf->surface->h,
        .pitch = buf->surface->pitch,
    };

    switch (buf->surface->format) {
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888: {
            data.format = PIXEL_FORMAT_RGBA_8888;
            break;
        }
        default:
            crash_msg("Unhandled surface format\n");
            break;
    }

    return data;
}

void swapchain_close(SwapchainBuffer* buf, SwapchainBufferData* data) {
    SDL_UnlockSurface(buf->surface);

    *data = (SwapchainBufferData){};
}

bool window_resize(WindowHandle* handle, iVec2 new_size) {
    i32 current_w, current_h;
    bool success = SDL_GetWindowSize(handle->window, &current_w, &current_h);
    assert(success);

    if (current_w != new_size.x || current_h != new_size.y) {
        success = SDL_SetWindowSize(handle->window, new_size.x, new_size.y);
        if (success) {
            current_w = new_size.x;
            current_h = new_size.y;
        }
    }

    handle->window_size = (iVec2){current_w, current_h};
    handle->window_surface = SDL_GetWindowSurface(handle->window);
    SDL_UpdateWindowSurface(handle->window);

    worker_destroy(handle->swapchain.presenter);

    for (i32 swapchain_idx = 0; swapchain_idx < handle->swapchain.count; swapchain_idx++) {
        SwapchainBuffer* swapchain_buf = &handle->swapchain.bufs[swapchain_idx];
        SDL_DestroySurface(swapchain_buf->surface);

        swapchain_buf->surface = SDL_CreateSurface(
            handle->window_surface->w,
            handle->window_surface->h,
            handle->window_surface->format
        );

        assert(swapchain_buf->surface);

        atomic_store(&swapchain_buf->state, SWAPCHAIN_BUFFER_STATE_FREE);
    }

    WorkerCreateInfo presenter_info = {
        .task_q_cap = kMaxSwapchainBuffers,
    };
    handle->swapchain.presenter = worker_create(&presenter_info);

    return success;
}

bool window_matches_id(WindowHandle* handle, WindowHandleId id) {
    return handle->id == (SDL_WindowID)id;
}
