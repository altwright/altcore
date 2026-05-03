//
// Created by wright on 5/2/26.
//

#ifndef ALTCORE_WINDOW_H
#define ALTCORE_WINDOW_H

#include "types.h"
#include "draw/framebuffer.h"

struct WINDOW_HANDLE_T;
typedef struct WINDOW_HANDLE_T WindowHandle;

typedef enum WINDOW_MODE_E {
#ifndef X_WINDOW_MODES
#define X_WINDOW_MODES \
    X(WINDOWED) \
    X(FULLSCREEN) \
    X(BORDERLESS) \
    X(COUNT)
#endif
#ifndef X
#define X(mode) \
    WINDOW_MODE_##mode,
#endif
    X_WINDOW_MODES
#undef X
} WindowMode;

typedef enum WINDOW_FLAG_OPTION_E {
#ifndef X_WINDOW_FLAG_OPTIONS
#define X_WINDOW_FLAG_OPTIONS \
    X(RESIZABLE) \
    X(DISABLE_VSYNC) \
    X(COUNT)
#endif
#ifndef X
#define X(option) \
    WINDOW_FLAG_OPTION_##option,
#endif
    X_WINDOW_FLAG_OPTIONS
#undef X
} WindowFlagOption;

typedef enum WINDOW_FLAG_E : u64 {
#ifndef X
#define X(flag) \
    WINDOW_FLAG_##flag = 1ULL << WINDOW_FLAG_OPTION_##flag,
#endif
    X_WINDOW_FLAG_OPTIONS
#undef X
} WindowFlag;

typedef u64 WindowFlags;

typedef enum SWAPCHAIN_MODE_E {
#ifndef X_SWAPCHAIN_MODES
#define X_SWAPCHAIN_MODES \
    X(DOUBLE_BUFFERED) \
    X(TRIPLE_BUFFERED) \
    X(COUNT)
#endif
#ifndef X
#define X(mode) \
    SWAPCHAIN_MODE_##mode,
#endif
    X_SWAPCHAIN_MODES
#undef X
} SwapchainMode;

typedef struct WINDOW_CREATE_INFO_T {
    const char* title;
    iVec2 size;
    iVec2 pos;
    i32 display_idx;
    WindowMode mode;
    WindowFlags flags;
    SwapchainMode swapchain_mode;
} WindowCreateInfo;

typedef struct DISPLAY_INFO_T {
    iVec2 windowed_size;
    iVec2 fullscreen_size;
} DisplayInfo;

typedef struct DISPLAY_INFOS_T {
    ARRAY_FIELDS(DisplayInfo)
} DisplayInfos;

void window_get_display_infos(DisplayInfos *out);

WindowHandle* window_create(const WindowCreateInfo* info);

void window_destroy(WindowHandle* handle);

i32 window_get_framebuffer_count(WindowHandle* handle);

Framebuffer* window_get_next_framebuffer();

void window_present_framebuffer(WindowHandle* handle, Framebuffer* buf);

#endif //ALTCORE_WINDOW_H
