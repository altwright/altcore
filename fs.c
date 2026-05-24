//
// Created by wright on 5/23/26.
//

#include "fs.h"

#include <stdio.h>
#include <assert.h>

#include "cwalk.h"
#include "memory.h"
#include "debug.h"

struct FILEPATH_T {
    char *abs;
    i64 abs_len;
    FilepathOrigin origin;
};

struct FILE_HANDLE_T {
    FILE* fp;
};

Filepath *fs_path_create(const string *path, FilepathOrigin origin) {
    Filepath *path_handle = nullptr;

    size_t normalized_len = cwk_path_normalize(path->data, nullptr, 0);
    char *normalized_buf = alt_calloc(normalized_len + 1, sizeof(char));
    assert(normalized_buf);

    cwk_path_normalize(path->data, normalized_buf, normalized_len + 1);

    path_handle = alt_malloc(sizeof(*path_handle));
    assert(path_handle);

    *path_handle = (Filepath){
        .abs = normalized_buf,
        .abs_len = (i64) normalized_len,
    };

    return path_handle;
}

string fs_path_get_abs(Arena* arena, Filepath* path) {
    return str_make(arena, "%.*s", path->abs_len, path->abs);
}

void fs_path_destroy(Filepath *path) {
    alt_free(path->abs);
    path->abs_len = 0;
    alt_free(path);
}

FileHandle *fs_file_open(Filepath *path, FileHandleMode mode) {
    FileHandle* handle = nullptr;

    switch (mode) {
        case FILE_HANDLE_MODE_BINARY_APPEND:
        case FILE_HANDLE_MODE_BINARY_WRITE: {
            if (path->origin == FILEPATH_ORIGIN_ROM) {
                crash_msg("Cannot write into ROM with path %s\n", path->abs);
            }
            break;
        }
        default:
            break;
    }

    const char* open_mode = nullptr;

    switch (mode) {
        case FILE_HANDLE_MODE_BINARY_READ: {
            open_mode = "rb";
            break;
        }
        case FILE_HANDLE_MODE_BINARY_WRITE: {
            open_mode = "wb";
            break;
        }
        case FILE_HANDLE_MODE_BINARY_APPEND: {
            open_mode = "ab";
            break;
        }
        default:
            crash_msg("Unhandled file open mode %d\n", mode);
            break;
    }

    FILE* fp = fopen(path->abs, open_mode);
    if (!fp) {
        return nullptr;
    }

    handle = alt_malloc(sizeof(*handle));

    *handle = (FileHandle){
        .fp = fp
    };

    return handle;
}

void fs_file_close(FileHandle *file) {
    int err = fclose(file->fp);
    assert(!err);

    file->fp = nullptr;

    alt_free(file);
}

u8s fs_file_to_buf(Arena* arena, FileHandle *file) {
    fseek(file->fp, 0, SEEK_END);
    long fp_size = ftell(file->fp);
    rewind(file->fp);

    u8s bytes = {arena, fp_size};
    ARRAY_MAKE(&bytes);

    fread(bytes.data, sizeof(u8), fp_size, file->fp);

    return bytes;
}
