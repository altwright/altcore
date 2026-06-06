//
// Created by wright on 6/5/26.
//

#include "ecs.h"

#include "arenas.h"
#include "hashmap.h"
#include "debug.h"

typedef struct ENTITY_VARS_T {
    ARRAY_FIELDS(EntityVar)
} EntityVars;

typedef struct ENTITY_T {
    EntityID eid;
    u64 start_tick;
    const char *name;
    EntityComponentFlags components;
    i32 tick_fn_ptr_idx;
    EntityVars vars;
    u64 priority;
} Entity;

typedef struct ENTITY_MAP_T {
    HASHMAP_FIELDS(EntityID, Entity)
} EntityMap;

typedef struct ENTITY_PTRS_T {
    ARRAY_FIELDS(Entity*)
} EntityPtrs;

typedef struct ENTITY_TICK_FN_PTRS_T {
    ARRAY_FIELDS(EntityTickFnPtr)
} EntityTickFnPtrs;

typedef struct ENTITY_IDS_T {
    ARRAY_FIELDS(EntityID)
} EntityIDs;

#ifndef COMPONENT_ARRAY_FIELDS
#define COMPONENT_ARRAY_FIELDS(component_type) \
component_type* data; \
EntityID* eids; \
i64 len; \
i64 cap; \
Arena* arena;
#endif

#ifndef COMPONENT_ARRAY_ADD
#define COMPONENT_ARRAY_ADD(component_array_ptr, eid_ptr) \
do { \
    component_array_add( \
        (void **)(&((component_array_ptr)->data)), \
        sizeof((component_array_ptr)->data[0]), \
        &((component_array_ptr)->eids), \
        &((component_array_ptr)->len), \
        &((component_array_ptr)->cap), \
        &((component_array_ptr)->arena), \
        (eid_ptr) \
    ); \
} while(0)
#endif

#ifndef COMPONENT_ARRAY_DEL
#define COMPONENT_ARRAY_DEL(component_array_ptr, eid_ptr) \
do { \
    component_array_del( \
        (component_array_ptr)->data, \
        sizeof(*((component_array_ptr)->data)), \
        (component_array_ptr)->eids, \
        &((component_array_ptr)->len), \
        (eid_ptr) \
    ); \
} while(0)
#endif

#ifndef COMPONENT_ARRAY_FREE
#define COMPONENT_ARRAY_FREE(component_array_ptr) \
do { \
    component_array_free( \
        (void **)(&((component_array_ptr)->data)), \
        &((component_array_ptr)->eids), \
        &((component_array_ptr)->len), \
        &((component_array_ptr)->cap), \
        &((component_array_ptr)->arena) \
    ); \
} while(0)
#endif

typedef struct F32X4_COMPONENTS_T {
    COMPONENT_ARRAY_FIELDS(f32x4)
} F32x4Components;

typedef struct F32X3_COMPONENTS_T {
    COMPONENT_ARRAY_FIELDS(f32x3)
} F32x3Components;

static bool g_initialized = false;
static u64 g_tick_counter = 0;
static u64 g_entity_counter = 0;
static EntityMap g_entity_map = {};
static EntityPtrs g_entity_ptrs = {};
static EntityTickFnPtrs g_entity_tick_fn_ptrs = {};

static F32x3Components g_positions = {};
static F32x4Components g_rotations = {};
static F32x3Components g_scales = {};

constexpr i64 kDefaultEntitiesCapacity = 256;

static void component_array_add(
    void **data,
    u64 data_elem_size,
    EntityID **eids,
    i64 *len,
    i64 *cap,
    Arena **arena,
    const EntityID *new_eid
) {
}

static void component_array_del(
    void *data,
    u64 data_elem_size,
    EntityID *eids,
    i64 *len,
    const EntityID *del_eid
) {
}

static void component_array_free(
    void **data,
    EntityID **eids,
    i64 *len,
    i64 *cap,
    Arena **arena
) {
}

void ecs_init() {
    if (g_initialized) {
        return;
    }

    g_entity_map = (EntityMap){HASHMAP_TYPE_NON_STR_KEY, HASHMAP_DEL_FREQ_HIGH};

    Entity default_entity = {
        .eid = {-1}
    };
    HASHMAP_MAKE(&g_entity_map, &default_entity);

    g_entity_ptrs = (EntityPtrs){
        .arena = arena_make(kDefaultEntitiesCapacity * sizeof(*g_entity_ptrs.data)),
        .cap = kDefaultEntitiesCapacity
    };

    ARRAY_MAKE(&g_entity_ptrs);

    g_tick_counter = 0;

    g_entity_counter = 0;

    g_initialized = true;
}

void ecs_deinit() {
    if (!g_initialized) {
        return;
    }

    for (i32 component_idx = 0; component_idx < ENTITY_COMPONENT_INDEX_COUNT; component_idx++) {
        switch ((EntityComponentIndex) component_idx) {
            case ENTITY_COMPONENT_INDEX_POSITION: {
                COMPONENT_ARRAY_FREE(&g_positions);
                break;
            }
            case ENTITY_COMPONENT_INDEX_ROTATION: {
                COMPONENT_ARRAY_FREE(&g_rotations);
                break;
            }
            case ENTITY_COMPONENT_INDEX_SCALE: {
                COMPONENT_ARRAY_FREE(&g_scales);
                break;
            }
            default:
                crash_msg("Unhandled component type index %d deinit\n", component_idx);
                break;
        }
    }

    arena_free(g_entity_ptrs.arena);

    g_entity_ptrs = (EntityPtrs){};

    HASHMAP_FREE(&g_entity_map);

    g_initialized = false;
}

void ecs_set_tick_fn_ptrs(EntityTickFnPtr *tick_fn_ptrs, i32 tick_fn_ptrs_len) {
    if (g_entity_tick_fn_ptrs.arena) {
        arena_free(g_entity_tick_fn_ptrs.arena);
    }

    i64 array_size = tick_fn_ptrs_len * (i64) sizeof(*g_entity_tick_fn_ptrs.data);
    g_entity_tick_fn_ptrs = (EntityTickFnPtrs){
        .arena = arena_make(array_size),
        .len = tick_fn_ptrs_len
    };

    ARRAY_MAKE(&g_entity_tick_fn_ptrs);

    memcpy(g_entity_tick_fn_ptrs.data, tick_fn_ptrs, array_size);
}

void ecs_tick() {
    if (!g_initialized) {
        return;
    }

    for (i64 entity_idx = 0; entity_idx < g_entity_ptrs.len; entity_idx++) {
        Entity *entity = *ARRAY_GET(&g_entity_ptrs, entity_idx);

        if (entity->tick_fn_ptr_idx >= 0) {
            EntityTickFnPtr entity_tick_fn_ptr = *ARRAY_GET(&g_entity_tick_fn_ptrs, entity->tick_fn_ptr_idx);

            EntityTickReturnCode code = entity_tick_fn_ptr(entity->eid);

            switch (code) {
                case ENTITY_TICK_RETURN_CODE_EXIT: {
                    EntityID eid = entity->eid;
                    EntityComponentFlags components = entity->components;

                    for (i64 component_idx = 0; component_idx < ENTITY_COMPONENT_INDEX_COUNT; component_idx++) {
                        EntityComponentFlag component_flag = 1ULL << component_idx;
                        if (components & component_flag) {
                            switch (component_flag) {
                                case ENTITY_COMPONENT_FLAG_POSITION: {
                                    COMPONENT_ARRAY_DEL(&g_positions, &eid);
                                    break;
                                }
                                case ENTITY_COMPONENT_FLAG_ROTATION: {
                                    COMPONENT_ARRAY_DEL(&g_rotations, &eid);
                                    break;
                                }
                                case ENTITY_COMPONENT_FLAG_SCALE: {
                                    COMPONENT_ARRAY_DEL(&g_scales, &eid);
                                    break;
                                }
                                default:
                                    crash_msg("Unhandled component type %lu deletion\n", component_flag);
                                    break;
                            }
                        }
                    }

                    ARRAY_DEL(&g_entity_ptrs, entity_idx);
                    entity_idx--;

                    HASHMAP_DEL(&g_entity_map, &eid);

                    break;
                }
                default:
                    break;
            }
        }
    }

    g_tick_counter++;
}

EntityID ecs_create_entity(const EntityCreateInfo *info) {
    EntityID new_eid = {(i64)++g_entity_counter};

    u64 vars_size = info->vars.len * sizeof(*info->vars.data);

    Entity new_entity = {
        .eid = new_eid,
        .start_tick = g_tick_counter,
        .name = info->name,
        .components = info->components,
        .tick_fn_ptr_idx = info->tick_fn_ptr_idx,
        .priority = info->priority,
        .vars = {
            .arena = arena_make((i64)vars_size),
            .len = info->vars.len,
        },
    };

    ARRAY_MAKE(&new_entity.vars);
    memcpy(new_entity.vars.data, info->vars.data, vars_size);

    for (i32 component_idx = 0; component_idx < ENTITY_COMPONENT_INDEX_COUNT; component_idx++) {
        EntityComponentFlag component_flag = 1ULL << component_idx;
        if (info->components & component_flag) {
            switch (component_flag) {
                case ENTITY_COMPONENT_FLAG_POSITION: {
                    COMPONENT_ARRAY_ADD(&g_positions, &new_eid);
                    break;
                }
                case ENTITY_COMPONENT_FLAG_ROTATION: {
                    COMPONENT_ARRAY_ADD(&g_rotations, &new_eid);
                    break;
                }
                case ENTITY_COMPONENT_FLAG_SCALE: {
                    COMPONENT_ARRAY_ADD(&g_scales, &new_eid);
                    break;
                }
                default:
                    crash_msg("Unhandled component flag add %lu\n", component_flag);
                    break;
            }
        }
    }

    return new_eid;
}
