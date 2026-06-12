//
// Created by wright on 6/5/26.
//

#include "ecs.h"

#include "arenas.h"
#include "hashmap.h"
#include "debug.h"
#include "maths.h"

typedef struct ENTITY_VARS_T {
    ARRAY_FIELDS(EntityVar)
} EntityVars;

typedef struct ENTITY_T {
    EntityID eid;
    u64 prev_tick;
    const char *name;
    EntityComponentFlags components;
    i64 fn_ptrs_idx;
    EntityVars vars;
    u64 priority;
} Entity;

typedef struct ENTITY_MAP_T {
    HASHMAP_FIELDS(EntityID, Entity)
} EntityMap;

typedef struct ENTITY_PTRS_T {
    ARRAY_FIELDS(Entity*)
} EntityPtrs;

typedef struct ENTITY_FN_PTRS_ARRAY_T {
    ARRAY_FIELDS(EntityFnPtrs)
} EntityFnPtrsArray;

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

#ifndef COMPONENT_ARRAY_PUT
#define COMPONENT_ARRAY_PUT(component_array_ptr, eid_ptr) \
    component_array_put( \
        (void **)(&((component_array_ptr)->data)), \
        sizeof(*((component_array_ptr)->data)), \
        &((component_array_ptr)->eids), \
        &((component_array_ptr)->len), \
        &((component_array_ptr)->cap), \
        &((component_array_ptr)->arena), \
        (eid_ptr) \
    );
#endif

#ifndef COMPONENT_ARRAY_DEL
#define COMPONENT_ARRAY_DEL(component_array_ptr, eid_ptr) \
    component_array_del( \
        (component_array_ptr)->data, \
        sizeof(*((component_array_ptr)->data)), \
        (component_array_ptr)->eids, \
        &((component_array_ptr)->len), \
        (eid_ptr) \
    )
#endif

#ifndef COMPONENT_ARRAY_FREE
#define COMPONENT_ARRAY_FREE(component_array_ptr) \
    component_array_free( \
        (void **)(&((component_array_ptr)->data)), \
        &((component_array_ptr)->eids), \
        &((component_array_ptr)->len), \
        &((component_array_ptr)->cap), \
        &((component_array_ptr)->arena) \
    )
#endif

#ifndef COMPONENT_ARRAY_GET
#define COMPONENT_ARRAY_GET(component_array_ptr, eid_ptr) \
    (typeof(*((component_array_ptr)->data))*) \
    component_array_get( \
        (component_array_ptr)->data, \
        sizeof(*((component_array_ptr)->data)), \
        (component_array_ptr)->eids, \
        (component_array_ptr)->len, \
        (eid_ptr) \
    )
#endif

typedef struct F32X4_COMPONENTS_T {
    COMPONENT_ARRAY_FIELDS(f32x4)
} F32x4Components;

typedef struct F32X3_COMPONENTS_T {
    COMPONENT_ARRAY_FIELDS(f32x3)
} F32x3Components;

typedef struct F32X44_COMPONENTS_T {
    COMPONENT_ARRAY_FIELDS(f32x44)
} F32x44Components;

typedef struct POINT_LIGHT_COMPONENTS_T {
    COMPONENT_ARRAY_FIELDS(PointLightComponent)
} PointLightComponents;

static bool g_initialized = false;
static u64 g_tick_counter = 0;
static u64 g_entity_counter = 0;
static EntityMap g_entity_map = {};
static EntityPtrs g_entity_ptrs = {};
static EntityFnPtrsArray g_entity_fn_ptrs_array = {};

static F32x3Components g_positions = {};
static F32x4Components g_rotations = {};
static F32x3Components g_scales = {};
static F32x44Components g_transform_3ds = {};
static PointLightComponents g_point_lights = {};

constexpr i64 kDefaultEntitiesCapacity = 256;

static void component_array_extend(
    void **data_ptr,
    u64 data_elem_size,
    EntityID **eids_ptr,
    i64 len,
    i64 *cap,
    Arena **arena_ptr
) {
    i64 new_cap = 2 * (*cap);
    if (new_cap <= 0) {
        new_cap = kDefaultEntitiesCapacity;
    }

    Arena *new_arena = arena_make(new_cap * (i64) (data_elem_size + sizeof(EntityID)));
    EntityID *new_eids = arena_alloc(new_arena, new_cap * (i64) sizeof(EntityID));
    void *new_data = arena_alloc(new_arena, new_cap * (i64) data_elem_size);

    if (*arena_ptr) {
        memcpy(new_data, *data_ptr, len * data_elem_size);
        memcpy(new_eids, *eids_ptr, len * sizeof(EntityID));
    }

    arena_free(*arena_ptr);

    *arena_ptr = new_arena;
    *data_ptr = new_data;
    *eids_ptr = new_eids;
    *cap = new_cap;
}

static void component_array_put(
    void **data_ptr,
    u64 data_elem_size,
    EntityID **eids_ptr,
    i64 *len,
    i64 *cap,
    Arena **arena_ptr,
    const EntityID *new_eid
) {
    while (*len >= *cap) {
        component_array_extend(data_ptr, data_elem_size, eids_ptr, *len, cap, arena_ptr);
    }

    i64 put_idx = 0;

    for (i64 current_idx = *len - 1; current_idx >= 0; current_idx--) {
        EntityID *current_eid = *eids_ptr + current_idx;
        if (current_eid->guid < new_eid->guid) {
            put_idx = current_idx + 1;
            break;
        }
    }

    for (i64 current_idx = *len; current_idx > put_idx; current_idx--) {
        EntityID *current_eid = *eids_ptr + current_idx;
        EntityID *prev_eid = current_eid - 1;
        memcpy(current_eid, prev_eid, sizeof(EntityID));

        u8 *current_data_start = ((u8 *) *data_ptr) + current_idx * data_elem_size;
        u8 *prev_data_start = current_data_start - data_elem_size;
        memcpy(current_data_start, prev_data_start, data_elem_size);
    }

    memcpy((*eids_ptr) + put_idx, new_eid, sizeof(EntityID));
    memset((u8 *) (*data_ptr) + put_idx * data_elem_size, 0, data_elem_size);

    (*len)++;
}

static void *component_array_get(
    void *data,
    u64 data_elem_size,
    EntityID *eids,
    i64 len,
    const EntityID *eid
) {
    u8 *elem_start = nullptr;

    u64 search_guid = eid->guid;

    i64 start_idx = 0;
    i64 end_idx = len;

    while (start_idx < end_idx) {
        i64 middle_idx = start_idx + (end_idx - start_idx) / 2;

        u64 middle_guid = eids[middle_idx].guid;

        if (middle_guid < search_guid) {
            start_idx = middle_idx + 1;
        } else if (middle_guid > search_guid) {
            end_idx = middle_idx - 1;
        } else {
            elem_start = (u8 *) data + (middle_idx * data_elem_size);
            break;
        }
    }

    return elem_start;
}

static void component_array_del(
    void *data,
    u64 data_elem_size,
    EntityID *eids,
    i64 *len,
    const EntityID *del_eid
) {
    void *del_elem = component_array_get(data, data_elem_size, eids, *len, del_eid);
    if (!del_elem) {
        return;
    }

    i64 del_elem_idx = ((u8 *) del_elem - (u8 *) data) / (i64) data_elem_size;

    for (i64 current_elem_idx = del_elem_idx; current_elem_idx < (*len) - 1; current_elem_idx++) {
        u8 *current_elem = (u8 *) data + (current_elem_idx * data_elem_size);
        u8 *next_elem = current_elem + data_elem_size;

        memcpy(current_elem, next_elem, data_elem_size);

        memcpy(&eids[current_elem_idx], &eids[current_elem_idx + 1], sizeof(*eids));
    }

    (*len)--;
}

static void component_array_free(
    void **data,
    EntityID **eids,
    i64 *len,
    i64 *cap,
    Arena **arena
) {
    arena_free(*arena);
    *data = nullptr;
    *eids = nullptr;
    *len = *cap = 0;
}

static void entity_add_component(EntityID eid, EntityComponentFlag flag) {
    switch (flag) {
        case ENTITY_COMPONENT_FLAG_POSITION: {
            COMPONENT_ARRAY_PUT(&g_positions, &eid);
            break;
        }
        case ENTITY_COMPONENT_FLAG_ROTATION: {
            COMPONENT_ARRAY_PUT(&g_rotations, &eid);
            break;
        }
        case ENTITY_COMPONENT_FLAG_SCALE: {
            COMPONENT_ARRAY_PUT(&g_scales, &eid);
            break;
        }
        case ENTITY_COMPONENT_FLAG_TRANSFORM_3D: {
            COMPONENT_ARRAY_PUT(&g_transform_3ds, &eid);
            break;
        }
        case ENTITY_COMPONENT_FLAG_POINT_LIGHT: {
            COMPONENT_ARRAY_PUT(&g_point_lights, &eid);
            break;
        }
        default:
            crash_msg("Unhandled component flag %lu addition\n", flag);
            break;
    }
}

static void entity_del_component(EntityID eid, EntityComponentFlag flag) {
    switch (flag) {
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
        case ENTITY_COMPONENT_FLAG_POINT_LIGHT: {
            COMPONENT_ARRAY_DEL(&g_point_lights, &eid);
            break;
        }
        case ENTITY_COMPONENT_FLAG_TRANSFORM_3D: {
            COMPONENT_ARRAY_DEL(&g_transform_3ds, &eid);
            break;
        }
        default:
            crash_msg("Unhandled component flag %lu deletion\n", flag);
            break;
    }
}

void ecs_init() {
    if (g_initialized) {
        return;
    }

    g_entity_map = (EntityMap){HASHMAP_TYPE_NON_STR_KEY, HASHMAP_DEL_FREQ_HIGH};

    Entity default_entity = {};
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
            case ENTITY_COMPONENT_INDEX_TRANSFORM_3D: {
                COMPONENT_ARRAY_FREE(&g_transform_3ds);
                break;
            }
            case ENTITY_COMPONENT_INDEX_POINT_LIGHT: {
                COMPONENT_ARRAY_FREE(&g_point_lights);
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

void ecs_set_entity_fn_ptrs(EntityFnPtrs *fn_ptrs, i32 fn_ptrs_len) {
    if (g_entity_fn_ptrs_array.arena) {
        arena_free(g_entity_fn_ptrs_array.arena);
    }

    i64 array_size = fn_ptrs_len * (i64) sizeof(*g_entity_fn_ptrs_array.data);
    g_entity_fn_ptrs_array = (EntityFnPtrsArray){
        .arena = arena_make(array_size),
        .len = fn_ptrs_len,
    };

    ARRAY_MAKE(&g_entity_fn_ptrs_array);

    memcpy(g_entity_fn_ptrs_array.data, fn_ptrs, array_size);
}

void ecs_tick() {
    if (!g_initialized) {
        return;
    }

    for (i64 entity_idx = 0; entity_idx < g_entity_ptrs.len; entity_idx++) {
        Entity *entity = *ARRAY_GET(&g_entity_ptrs, entity_idx);

        if (entity->fn_ptrs_idx >= 0 && entity->prev_tick < g_tick_counter) {
            entity->prev_tick = g_tick_counter;

            EntityTickFnPtr entity_tick_fn_ptr = ARRAY_GET(&g_entity_fn_ptrs_array, entity->fn_ptrs_idx)->tick_fn_ptr;

            EntityTickReturnCode code = entity_tick_fn_ptr(entity->eid, ENTITY_TICK_SIGNAL_NONE);

            switch (code) {
                case ENTITY_TICK_RETURN_CODE_EXIT: {
                    EntityComponentFlags components = entity->components;

                    for (i64 component_idx = 0; component_idx < ENTITY_COMPONENT_INDEX_COUNT; component_idx++) {
                        EntityComponentFlag component_flag = 1ULL << component_idx;
                        if (components & component_flag) {
                            entity_del_component(entity->eid, component_flag);
                        }
                    }

                    if (entity->vars.arena) {
                        arena_free(entity->vars.arena);
                    }

                    ARRAY_DEL(&g_entity_ptrs, entity_idx);
                    entity_idx--;

                    HASHMAP_DEL(&g_entity_map, &entity->eid);

                    break;
                }
                default:
                    break;
            }
        }
    }

    g_tick_counter++;
}


EntityID ecs_add_entity(const EntityCreateInfo *info) {
    EntityID new_eid = {++g_entity_counter};

    u64 vars_size = info->var_types.len * sizeof(EntityVar);
    Entity new_entity = {
        .eid = new_eid,
        .prev_tick = 0,
        .name = info->name,
        .components = info->components,
        .fn_ptrs_idx = info->entity_type_idx,
        .priority = info->priority,
    };

    if (info->var_types.len > 0) {
        new_entity.vars = (EntityVars){
            .arena = arena_make((i64) vars_size),
            .len = info->var_types.len,
        };
        ARRAY_MAKE(&new_entity.vars);
    }

    for (i32 var_idx = 0; var_idx < info->var_types.len; var_idx++) {
        EntityVar *var = ARRAY_GET(&new_entity.vars, var_idx);
        var->type = info->var_types.data[var_idx];
    }

    HASHMAP_PUT(&g_entity_map, &new_eid, &new_entity);

    while (g_entity_ptrs.len >= g_entity_ptrs.cap) {
        i64 new_cap = 2 * g_entity_ptrs.cap;
        u64 elem_size = sizeof(g_entity_ptrs.data[0]);
        u64 new_size = new_cap * elem_size;

        EntityPtrs new_entity_ptrs = {
            .arena = arena_make((i64) new_size),
            .len = g_entity_ptrs.len,
            .cap = new_cap,
        };

        ARRAY_MAKE(&new_entity_ptrs);
        memcpy(new_entity_ptrs.data, g_entity_ptrs.data, g_entity_ptrs.len * elem_size);

        arena_free(g_entity_ptrs.arena);

        g_entity_ptrs = new_entity_ptrs;
    }

    Entity *new_entity_ptr = &(HASHMAP_GET(&g_entity_map, &new_eid)->value);

    i64 new_entity_ptr_idx = 0;

    for (i64 entity_ptr_idx = 0; entity_ptr_idx < g_entity_ptrs.len; entity_ptr_idx++) {
        new_entity_ptr_idx = entity_ptr_idx;

        Entity *entity_ptr = *ARRAY_GET(&g_entity_ptrs, entity_ptr_idx);
        if ((entity_ptr->priority) > (new_entity_ptr->priority)) {
            break;
        }
    }

    ARRAY_PUT(&g_entity_ptrs, new_entity_ptr_idx, &new_entity_ptr);

    for (i32 component_idx = 0; component_idx < ENTITY_COMPONENT_INDEX_COUNT; component_idx++) {
        EntityComponentFlag component_flag = 1ULL << component_idx;
        if (info->components & component_flag) {
            entity_add_component(new_eid, component_flag);
        }
    }

    return new_eid;
}

bool ecs_entity_exists(EntityID eid) {
    return HASHMAP_GET(&g_entity_map, &eid);
}

static void *get_entity_var(EntityID eid, i32 var_idx, EntityVarType var_type) {
    void *var_ptr = nullptr;

    auto entity_pair = HASHMAP_GET(&g_entity_map, &eid);
    if (entity_pair) {
        Entity *entity = &entity_pair->value;

        EntityVar *var_elem = ARRAY_GET(&entity->vars, var_idx);

        if (var_type == var_elem->type) {
            switch (var_type) {
                case ENTITY_VAR_TYPE_I32: {
                    var_ptr = &var_elem->data.i32_val;
                    break;
                }
                case ENTITY_VAR_TYPE_U32: {
                    var_ptr = &var_elem->data.u32_val;
                    break;
                }
                case ENTITY_VAR_TYPE_F32: {
                    var_ptr = &var_elem->data.f32_val;
                    break;
                }
                case ENTITY_VAR_TYPE_I64: {
                    var_ptr = &var_elem->data.i64_val;
                    break;
                }
                case ENTITY_VAR_TYPE_U64: {
                    var_ptr = &var_elem->data.u64_val;
                    break;
                }
                case ENTITY_VAR_TYPE_F64: {
                    var_ptr = &var_elem->data.f64_val;
                    break;
                }
                case ENTITY_VAR_TYPE_PTR: {
                    var_ptr = &var_elem->data.ptr_val;
                    break;
                }
                case ENTITY_VAR_TYPE_EID: {
                    var_ptr = &var_elem->data.eid_val;
                    break;
                }
                default:
                    crash_msg("Unhandled entity var type %d\n", var_type);
                    break;
            }
        }
    }

    return var_ptr;
}

i32 *ecs_get_i32_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_I32);
}

u32 *ecs_get_u32_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_U32);
}

f32 *ecs_get_f32_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_F32);
}

i64 *ecs_get_i64_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_I64);
}

u64 *ecs_get_u64_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_U64);
}

f64 *ecs_get_f64_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_F64);
}

void **ecs_get_ptr_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_PTR);
}

EntityID *ecs_get_eid_var(EntityID eid, i32 var_idx) {
    return get_entity_var(eid, var_idx, ENTITY_VAR_TYPE_EID);
}

EntityComponentFlags ecs_get_components(EntityID eid) {
    EntityComponentFlags flags = 0;

    auto entity_pair = HASHMAP_GET(&g_entity_map, &eid);
    if (entity_pair) {
        flags = entity_pair->value.components;
    }

    return flags;
}

void ecs_set_components(EntityID eid, EntityComponentFlags component_flags) {
    EntityComponentFlags current_flags = ecs_get_components(eid);
    EntityComponentFlags flags_diff = current_flags ^ component_flags;

    for (i32 component_idx = 0; component_idx < ENTITY_COMPONENT_INDEX_COUNT; component_idx++) {
        EntityComponentFlag component_flag = 1ULL << component_idx;
        if (flags_diff & component_flag) {
            if (component_flag & current_flags) {
                entity_del_component(eid, component_flag);
            } else {
                entity_add_component(eid, component_flag);
            }
        }
    }
}

u64 ecs_get_priority(EntityID eid) {
    u64 priority = UINT64_MAX;

    auto entity_pair = HASHMAP_GET(&g_entity_map, &eid);
    if (entity_pair) {
        priority = entity_pair->value.priority;
    }

    return priority;
}

f32x3 *ecs_get_entity_position(EntityID eid) {
    return COMPONENT_ARRAY_GET(&g_positions, &eid);
}

f32x4 *ecs_get_entity_rotation(EntityID eid) {
    return COMPONENT_ARRAY_GET(&g_rotations, &eid);
}

f32x3 *ecs_get_entity_scale(EntityID eid) {
    return COMPONENT_ARRAY_GET(&g_scales, &eid);
}

void ecs_get_positions(f32x3 **positions, EntityID **eids, i32 *len) {
    if (positions && eids && len) {
        *len = (i32) g_positions.len;
        *positions = g_positions.data;
        *eids = g_positions.eids;
    }
}

void ecs_get_rotations(f32x4 **rotations, EntityID **eids, i32 *len) {
    if (rotations && eids && len) {
        *len = (i32) g_rotations.len;
        *rotations = g_rotations.data;
        *eids = g_rotations.eids;
    }
}

void ecs_get_scales(f32x3 **scales, EntityID **eids, i32 *len) {
    if (scales && eids && len) {
        *len = (i32) g_scales.len;
        *scales = g_scales.data;
        *eids = g_scales.eids;
    }
}
