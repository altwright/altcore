//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_FS_H
#define ALTCORE_FS_H

#include "arenas.h"
#include "strings.h"

struct FILEPATH_T;
typedef struct FILEPATH_T Filepath;

typedef enum FILEPATH_ORIGIN_E {
#ifndef X_FILEPATH_ORIGINS
#define X_FILEPATH_ORIGINS \
    X(ROM) \
    X(SAVE) \
    X(COUNT)
#endif
#ifndef X
#define X(origin) \
    FILEPATH_ORIGIN_##origin,
#endif
    X_FILEPATH_ORIGINS
#undef X
} FilepathOrigin;

struct FILE_HANDLE_T;
typedef struct FILE_HANDLE_T FileHandle;

typedef enum FILE_HANDLE_MODE_E {
#ifndef X_FILE_HANDLE_MODES
#define X_FILE_HANDLE_MODES \
    X(BINARY_READ) \
    X(BINARY_WRITE) \
    X(BINARY_APPEND) \
    X(COUNT)
#endif
#ifndef X
#define X(mode) \
    FILE_HANDLE_MODE_##mode,
#endif
    X_FILE_HANDLE_MODES
#undef X
} FileHandleMode;

void fs_init();

void fs_deinit();

Filepath *fs_path_create(const string* path, FilepathOrigin origin);

string fs_path_get_abs(Arena* arena, Filepath* path);

void fs_path_destroy(Filepath *path);

FileHandle *fs_file_open(Filepath *path, FileHandleMode mode);

u8s fs_file_to_buf(Arena* arena, FileHandle *file);

void fs_file_close(FileHandle *file);

#endif //ALTCORE_FS_H
