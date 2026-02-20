//
// Created by wright on 2/19/26.
//

#include "malloc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct MALLOC_BUFFER_T {
    u8 *data;
    i64 cap;
    struct MALLOC_BUFFER_T *next;
} MallocBuffer;

typedef struct ALLOCATION_INFO_T {
    i64 used;
    i64 cap;
} AllocationInfo;

static MallocBuffer *g_buffer = nullptr;

static AllocationInfo create_empty_alloc_info() {
    return (AllocationInfo){
        .used = -1,
        .cap = -1
    };
}

void alt_init(i64 initial_cap) {
    if (!g_buffer) {
        g_buffer = calloc(1, sizeof(MallocBuffer));
        assert(g_buffer);
    }

    if (!g_buffer->data) {
        i64 aligned_cap = ((initial_cap + 3) >> 2) << 2;
        g_buffer->cap = (i64) sizeof(AllocationInfo) + aligned_cap;
        g_buffer->data = calloc(g_buffer->cap, sizeof(u8));
        assert(g_buffer->data);
    }

    AllocationInfo first_alloc_info = create_empty_alloc_info();
    memcpy(g_buffer->data, &first_alloc_info, sizeof(first_alloc_info));
}

void alt_uninit() {
    MallocBuffer *current_buffer = g_buffer;
    while (current_buffer) {
        free(current_buffer->data);
        current_buffer->data = nullptr;
        current_buffer->cap = 0;
        MallocBuffer *next_buffer = current_buffer->next;
        free(current_buffer);
        current_buffer = next_buffer;
    }

    g_buffer = nullptr;
}

static void create_next_buffer(MallocBuffer *current_buffer, i64 new_alloc_size) {
    assert(current_buffer && !current_buffer->next);

    MallocBuffer *next_malloc_buffer = calloc(1, sizeof(MallocBuffer));
    assert(next_malloc_buffer);

    new_alloc_size += (i64) sizeof(AllocationInfo); // For the alloc info at the beginning of
    // the new buffer.
    next_malloc_buffer->cap = new_alloc_size > current_buffer->cap
                                  ? new_alloc_size
                                  : current_buffer->cap;

    next_malloc_buffer->data = calloc(next_malloc_buffer->cap, sizeof(u8));
    assert(next_malloc_buffer->data);

    AllocationInfo *next_alloc_info = (AllocationInfo *) next_malloc_buffer->data;
    *next_alloc_info = create_empty_alloc_info();

    current_buffer->next = next_malloc_buffer;
}

void *alt_malloc(size_t size) {
    if (!g_buffer || !g_buffer->data || size <= 0) {
        return nullptr;
    }

    u8 *new_data = nullptr;

    i64 aligned_size = (((i64) size + 3) >> 2) << 2;

    MallocBuffer *current_buffer = g_buffer;

    AllocationInfo *current_alloc_info = (AllocationInfo *) current_buffer->data;

    while (!new_data) {
        if (current_alloc_info->cap < 0) {
            // Is an empty alloc
            i64 new_alloc_size = aligned_size + (i64) sizeof(AllocationInfo); // For the empty alloc info at the end

            // Offset of the beginning of the alloc data from the start of the buffer
            i64 current_alloc_data_offset = ((u8 *) current_alloc_info - current_buffer->data) +
                                            (i64) sizeof(AllocationInfo);

            if ((current_buffer->cap - current_alloc_data_offset) < new_alloc_size) {
                if (!current_buffer->next) {
                    create_next_buffer(current_buffer, new_alloc_size);
                }

                // Fill up the remainder of the undersized alloc, excluding the alloc info
                current_alloc_info->cap = current_buffer->cap - current_alloc_data_offset;
                current_buffer = current_buffer->next;;
            }

            current_alloc_info->cap = current_alloc_info->used = aligned_size;
            new_data = (u8 *) current_alloc_info + sizeof(AllocationInfo);

            AllocationInfo *next_alloc_info = (AllocationInfo *) (new_data + aligned_size);
            *next_alloc_info = create_empty_alloc_info();
        } else if (current_alloc_info->used < 0) {
            // Has been freed
            if (current_alloc_info->cap >= aligned_size) {
                current_alloc_info->used = aligned_size;
                new_data = (u8 *) current_alloc_info + sizeof(AllocationInfo);
            }
        }

        if (!new_data) {
            AllocationInfo *next_alloc_info = (AllocationInfo *) (
                (u8 *) current_alloc_info +
                sizeof(AllocationInfo) +
                current_alloc_info->cap
            );

            u8 *end_of_current_buffer = current_buffer->data + current_buffer->cap;

            assert((u8*)next_alloc_info + sizeof(AllocationInfo) <= end_of_current_buffer);

            if (next_alloc_info->cap >= 0) {
                // Beginning alloc info, cap, then following alloc info
                i64 valid_alloc_size = 2 * (i64) sizeof(AllocationInfo) + next_alloc_info->cap;
                if ((end_of_current_buffer - (u8 *) next_alloc_info) < (valid_alloc_size)) {
                    // There is no space for a valid alloc, therefore go to the next buffer
                    if (!current_buffer->next) {
                        i64 new_alloc_size = aligned_size + (i64) sizeof(AllocationInfo);
                        // For the empty alloc info at the end
                        create_next_buffer(current_buffer, new_alloc_size);
                    }
                    current_buffer = current_buffer->next;
                    next_alloc_info = (AllocationInfo *) current_buffer->data;
                }
            }

            current_alloc_info = next_alloc_info;
        }
    }

    return new_data;
}

void alt_free(void *ptr) {
    if (!ptr) {
        return;
    }

    u8 *alloc_data = ptr;

    AllocationInfo *alloc_info = ptr - sizeof(AllocationInfo);
    alloc_info->used = -1;
}

void *alt_calloc(size_t num_elems, size_t elem_size) {
    if (!g_buffer || !g_buffer->data || num_elems == 0 || elem_size == 0) {
        return nullptr;
    }

    void *new_data = alt_malloc(num_elems * elem_size);
    assert(new_data);

    memset(new_data, 0, num_elems * elem_size);

    return new_data;
}

void *alt_realloc(void *ptr, size_t new_size) {
    if (!g_buffer || !g_buffer->data || !ptr || new_size == 0) {
        return nullptr;
    }

    u8 *alloc_data = ptr;
    AllocationInfo *alloc_info = (AllocationInfo *) (alloc_data - sizeof(AllocationInfo));
    assert(alloc_info->cap >= 0 && alloc_info->used >= 0);

    i64 copy_size = alloc_info->used > new_size ? (i64) new_size : alloc_info->used;

    void *new_data = alt_malloc(new_size);

    memcpy(new_data, alloc_data, copy_size);

    alt_free(alloc_data);

    return new_data;
}
