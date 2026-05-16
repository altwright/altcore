//
// Created by wright on 5/2/26.
//

#include "window.h"

#include <assert.h>
#include <threads.h>

#include "SDL3/SDL_init.h"
#include <SDL3/SDL_video.h>

#include "../debug.h"
#include "../memory.h"
#include "framebuffer.h"
#include "framebuffer_impl.h"
#include "pixels_impl.h"

struct WINDOW_HANDLE_T {
    SDL_Window *window;
    SDL_WindowID id;
    iVec2 window_size;
    SDL_Surface *window_surface;
};

typedef struct {
    WindowHandle** data;
    i32 len;
    i32 cap;
    mtx_t lock;
} ActiveWindowHandles;

ActiveWindowHandles g_active_window_handles = {};

constexpr i32 kDefaultActiveWindowHandlesCap = 4;

static void init_g_active_window_handles() {
    g_active_window_handles.cap = kDefaultActiveWindowHandlesCap;
    g_active_window_handles.data = alt_calloc(
        g_active_window_handles.cap,
        sizeof(*g_active_window_handles.data)
    );
    g_active_window_handles.len = 0;
    mtx_init(&g_active_window_handles.lock, mtx_plain);
}

static void uninit_g_active_window_handles() {
    mtx_lock(&g_active_window_handles.lock);
    g_active_window_handles.len = 0;
    g_active_window_handles.cap = 0;
    alt_free(g_active_window_handles.data);
    g_active_window_handles.data = nullptr;
    mtx_unlock(&g_active_window_handles.lock);
    mtx_destroy(&g_active_window_handles.lock);
}

void window_get_display_infos(DisplayInfos *out) {
    if (!g_active_window_handles.data) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
        init_g_active_window_handles();
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

    if (!g_active_window_handles.data) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
        init_g_active_window_handles();
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

    handle->id = SDL_GetWindowID(handle->window);

    if (disable_vsync) {
        SDL_SetWindowSurfaceVSync(handle->window, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
    }

    bool success = SDL_GetWindowSize(handle->window, &handle->window_size.x, &handle->window_size.y);
    assert(success);

    handle->window_surface = SDL_GetWindowSurface(handle->window);
    SDL_UpdateWindowSurface(handle->window);

    mtx_lock(&g_active_window_handles.lock);

    if (g_active_window_handles.len >= g_active_window_handles.cap) {
        i32 new_cap = g_active_window_handles.cap * 2;
        WindowHandle** new_data = alt_calloc(new_cap, sizeof(*new_data));
        assert(new_data);
        memcpy(new_data, g_active_window_handles.data, sizeof(*new_data) * g_active_window_handles.len);
        alt_free(g_active_window_handles.data);
        g_active_window_handles.data = new_data;
    }

    g_active_window_handles.data[g_active_window_handles.len++] = handle;

    mtx_unlock(&g_active_window_handles.lock);

    return handle;
}

void window_destroy(WindowHandle *handle) {
    assert(g_active_window_handles.data);

    mtx_lock(&g_active_window_handles.lock);

    SDL_DestroyWindow(handle->window);

    alt_free(handle);

    g_active_window_handles.data[--g_active_window_handles.len] = nullptr;

    mtx_unlock(&g_active_window_handles.lock);

    if (g_active_window_handles.len <= 0) {
        uninit_g_active_window_handles();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

bool window_resize(WindowHandle *handle, iVec2 new_size) {
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

    return success;
}

WindowHandleInfo window_get_info(WindowHandle* handle) {
    WindowHandleInfo info = {
    };

    return info;
}

WindowHandle *window_impl_get_handle_from_id(SDL_WindowID id) {
    WindowHandle *matching_handle = nullptr;

    for (i32 handle_idx = 0; handle_idx < g_active_window_handles.len; handle_idx++) {
        WindowHandle *handle = g_active_window_handles.data[handle_idx];

        if (handle->id == id) {
            matching_handle = handle;
            break;
        }
    }

    return matching_handle;
}

void window_impl_update_surfaces() {
    mtx_lock(&g_active_window_handles.lock);
    for (i32 window_idx = 0; window_idx < g_active_window_handles.len; window_idx++) {
        // Needs to be called on the main thread
        SDL_UpdateWindowSurface(g_active_window_handles.data[window_idx]->window);
    }
    mtx_unlock(&g_active_window_handles.lock);
}

void window_present_framebuffer(WindowHandle* handle, Framebuffer* buf) {
    mtx_lock(&g_active_window_handles.lock);

    FramebufferInfo info = framebuffer_get_info(buf);
    assert(info.type == FRAMEBUFFER_TYPE_PIXEL);

    u8* bytes = framebuffer_impl_get_bytes(buf);

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        info.data.pixel_buf.size.x,
        info.data.pixel_buf.size.y,
        pixels_impl_to_sdl_format(info.data.pixel_buf.format),
        bytes,
        (i32)info.data.pixel_buf.pitch_bytes
    );

    SDL_BlitSurface(surface, nullptr, handle->window_surface, nullptr);

    SDL_DestroySurface(surface);

    mtx_unlock(&g_active_window_handles.lock);
}
