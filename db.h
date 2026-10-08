//
// Created by wright on 9/11/26.
//

#ifndef ALTCORE_DB_H
#define ALTCORE_DB_H

#include "types.h"
#include "arenas.h"

typedef enum DB_MOUNT_E : u32 {
#define X_DB_MOUNTS \
    X(RO) \
    X(RW) \
    X(COUNT)
#define X(mount) \
    DB_MOUNT_##mount,
    X_DB_MOUNTS
#undef X
} DbMount;

typedef struct DB_FOLDER_T {
    const char **nested_path;
    i64 nested_path_len;
} DbFolder;

typedef struct DB_READ_INFO_T {
    DbMount mount;
    DbFolder folder;
    const char *filename;
} DbReadInfo;

typedef struct DB_WRITE_INFO_T {
    DbFolder folder;
    const char *filename;

    struct {
        const u8 *data;
        i64 len;
    } bytes;
} DbWriteInfo;

typedef struct DB_READ_HANDLE_T DbReadHandle;

typedef struct DB_WRITE_HANDLE_T DbWriteHandle;

void db_init();

void db_deinit();

bool db_exists(const DbReadInfo *info);

i64 db_size(const DbReadInfo *info);

u8s db_read(Arena *arena, const DbReadInfo *info);

void db_write(const DbWriteInfo *info);

DbReadHandle *db_read_open(const DbReadInfo *info);

u64 db_read_next(DbReadHandle *handle, u8 *out_bytes, u64 out_bytes_len);

void db_read_close(DbReadHandle *handle);

/*
 * The size of the byte buffer to be written is read from
 * the info parameter - none of the contents of the byte
 * buffer are copied during the stream handle creation.
 */
DbWriteHandle *db_write_open(const DbWriteInfo *info);

u64 db_write_next(DbWriteHandle *handle, const u8 *in_bytes, u64 in_bytes_len);

void db_write_close(DbWriteHandle *handle);

#endif //ALTCORE_DB_H
