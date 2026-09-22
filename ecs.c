//
// Created by wright on 6/5/26.
//

#include "ecs.h"

#include "arenas.h"
#include "hashmap.h"
#include "debug.h"
#include "maths.h"
#include "memory.h"

constexpr i64 kDefaultEntitiesCapacity = 256;

typedef bits EntityComponentFlags;

typedef struct ENTITY_VARS_T {
    ARRAY_FIELDS(EntityVar)
} EntityVars;

typedef struct ENTITY_T {
    EntityID eid;
    u64 prev_tick;
    const char *name;
    i64 fn_ptrs_idx;
    EntityVars vars;
    EntityComponentFlags component_flags;
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

typedef struct COMPONENT_ARRAY_T {
    EntityID *eids;
    u8 *elems;
    u64 elem_size;
    i64 len;
    i64 cap;
    Arena *arena;
} ComponentArray;

typedef struct COMPONENT_ARRAYS_T {
    ARRAY_FIELDS(ComponentArray)
} ComponentArrays;

struct ECS_HANDLE_T {
    Arena* arena;
    u64 tick_counter;
    u64 entity_counter;
    EntityMap entity_map;
    EntityPtrs entity_ptrs;
    EntityFnPtrsArray entity_fn_ptrs_array;
    ComponentArrays component_arrays;
};

static void *component_array_get(
    const ComponentArray *component_array,
    const EntityID *eid
) {
    u8 *elem_start = nullptr;

    u64 search_guid = eid->guid;

    i64 start_idx = 0;
    i64 end_idx = component_array->len;

    while (start_idx < end_idx) {
        i64 middle_idx = start_idx + (end_idx - start_idx) / 2;

        u64 middle_guid = component_array->eids[middle_idx].guid;

        if (middle_guid < search_guid) {
            start_idx = middle_idx + 1;
        } else if (middle_guid > search_guid) {
            end_idx = middle_idx - 1;
        } else {
            elem_start = component_array->elems + (middle_idx * component_array->elem_size);
            break;
        }
    }

    return elem_start;
}

static void component_array_extend(ComponentArray *component_array) {
    i64 new_cap = 2 * (component_array->cap);
    if (new_cap <= 0) {
        new_cap = kDefaultEntitiesCapacity;
    }

    Arena *new_arena = arena_make(new_cap * (i64) (component_array->elem_size + sizeof(EntityID)));
    EntityID *new_eids = arena_alloc(new_arena, new_cap * (i64) sizeof(EntityID));
    void *new_elems = arena_alloc(new_arena, new_cap * (i64) component_array->elem_size);

    if (component_array->arena) {
        memcpy(new_elems, component_array->elems, component_array->len * component_array->elem_size);
        memcpy(new_eids, component_array->eids, component_array->len * sizeof(EntityID));
        arena_free(component_array->arena);
    }

    component_array->arena = new_arena;
    component_array->elems = new_elems;
    component_array->eids = new_eids;
    component_array->cap = new_cap;
}

static void component_array_put(
    ComponentArray *component_array,
    const EntityID *new_eid
) {
    void *del_elem = component_array_get(component_array, new_eid);
    if (del_elem) {
        return;
    }

    while (component_array->len >= component_array->cap) {
        component_array_extend(component_array);
    }

    i64 put_idx = 0;

    for (i64 current_idx = component_array->len - 1; current_idx >= 0; current_idx--) {
        EntityID *current_eid = component_array->eids + current_idx;
        if (current_eid->guid < new_eid->guid) {
            put_idx = current_idx + 1;
            break;
        }
    }

    for (i64 current_idx = component_array->len; current_idx > put_idx; current_idx--) {
        EntityID *current_eid = component_array->eids + current_idx;
        EntityID *prev_eid = current_eid - 1;
        memcpy(current_eid, prev_eid, sizeof(EntityID));

        u8 *current_data_start = (component_array->elems) + current_idx * component_array->elem_size;
        u8 *prev_data_start = current_data_start - component_array->elem_size;
        memcpy(current_data_start, prev_data_start, component_array->elem_size);
    }

    memcpy(component_array->eids + put_idx, new_eid, sizeof(EntityID));
    memset(component_array->elems + put_idx * component_array->elem_size, 0, component_array->elem_size);

    component_array->len++;
}


static void component_array_del(
    ComponentArray *component_array,
    const EntityID *del_eid
) {
    void *del_elem = component_array_get(component_array, del_eid);
    if (!del_elem) {
        return;
    }

    i64 del_elem_idx = ((u8 *) del_elem - component_array->elems) / (i64) component_array->elem_size;

    for (i64 current_elem_idx = del_elem_idx; current_elem_idx < component_array->len - 1; current_elem_idx++) {
        u8 *current_elem = component_array->elems + (current_elem_idx * component_array->elem_size);
        u8 *next_elem = current_elem + component_array->elem_size;

        memcpy(current_elem, next_elem, component_array->elem_size);

        memcpy(
            &component_array->eids[current_elem_idx],
            &component_array->eids[current_elem_idx + 1],
            sizeof(EntityID)
        );
    }

    component_array->len--;
}

static void component_array_free(ComponentArray* component_array) {
    if (component_array->arena) {
        arena_free(component_array->arena);
    }
    component_array->elems = nullptr;
    component_array->eids = nullptr;
    component_array->len = component_array->cap = 0;
}

EcsHandle *ecs_create(const EcsCreateInfo *info) {
    EcsHandle *ecs = alt_malloc(sizeof(*ecs));

    *ecs = (EcsHandle){
        .arena = arena_make(64 * MIBIBYTE),
        .tick_counter = 0,
        .entity_counter = 0,
    };

    ecs->entity_map = (EntityMap){
        .type = HASHMAP_TYPE_NON_STR_KEY,
        .del_freq = HASHMAP_DEL_FREQ_HIGH
    };

    Entity default_entity = {};
    HASHMAP_MAKE(&ecs->entity_map, &default_entity);

    ecs->entity_ptrs = (EntityPtrs){
        .arena = ecs->arena,
        .cap = kDefaultEntitiesCapacity
    };
    ARRAY_MAKE(&ecs->entity_ptrs);

    ecs->component_arrays = (ComponentArrays){
        .arena = ecs->arena,
        .len = info->component_types.len
    };
    ARRAY_MAKE(&ecs->component_arrays);

    for (i64 component_array_idx = 0; component_array_idx < info->component_types.len; component_array_idx++) {
        EntityComponentType component_type = info->component_types.data[component_array_idx];

        u64 elem_size = 0;

        switch (component_type) {
            case ENTITY_COMPONENT_TYPE_I32: {
                elem_size = sizeof(i32);
                break;
            }
            case ENTITY_COMPONENT_TYPE_F32X3: {
                elem_size = sizeof(f32x3);
                break;
            }
            default:
                crash_msg("Unhandled component type %d size\n", component_type);
                break;
        }

        ComponentArray *component_array = ARRAY_GET(&ecs->component_arrays, component_array_idx);
        *component_array = (ComponentArray){
            .elem_size = elem_size,
        };
    }

    return ecs;
}

void ecs_destroy(EcsHandle *ecs) {
    ARRAY_FOR(component_array, &ecs->component_arrays) {
        component_array_free(component_array);
    }

    HASHMAP_FREE(&ecs->entity_map);

    if (ecs->entity_fn_ptrs_array.arena) {
        arena_free(ecs->entity_fn_ptrs_array.arena);
    }

    arena_free(ecs->arena);
}

void ecs_set_entity_fn_ptrs(EcsHandle *ecs, EntityFnPtrs *fn_ptrs, i64 fn_ptrs_len) {
    if (ecs->entity_fn_ptrs_array.arena) {
        arena_free(ecs->entity_fn_ptrs_array.arena);
    }

    i64 array_size = fn_ptrs_len * (i64) sizeof(*ecs->entity_fn_ptrs_array.data);
    ecs->entity_fn_ptrs_array = (EntityFnPtrsArray){
        .arena = arena_make(array_size),
        .len = fn_ptrs_len,
    };

    ARRAY_MAKE(&ecs->entity_fn_ptrs_array);

    memcpy(ecs->entity_fn_ptrs_array.data, fn_ptrs, array_size);
}

void ecs_tick(EcsHandle *ecs) {
    ecs->tick_counter++;

    for (i64 entity_idx = 0; entity_idx < ecs->entity_ptrs.len; entity_idx++) {
        Entity *entity = *ARRAY_GET(&ecs->entity_ptrs, entity_idx);

        if (entity->fn_ptrs_idx >= 0 && entity->prev_tick < ecs->tick_counter) {
            entity->prev_tick = ecs->tick_counter;

            EntityTickFnPtr entity_tick_fn_ptr = ARRAY_GET(&ecs->entity_fn_ptrs_array, entity->fn_ptrs_idx)->
                    tick_fn_ptr;
            EntityTickReturnCode code = entity_tick_fn_ptr(ecs, entity->eid);

            switch (code) {
                case ENTITY_TICK_RETURN_CODE_EXIT: {
                    EntityComponentFlags *component_flags = &entity->component_flags;

                    if (component_flags->arena) {
                        for (i64 component_idx = 0; component_idx < ecs->component_arrays.len; component_idx++) {
                            if (bits_is_set(component_flags, component_idx)) {
                                ecs_entity_component_del(ecs, entity->eid, component_idx);
                            }
                        }
                    }

                    if (entity->vars.arena) {
                        arena_free(entity->vars.arena);
                    }

                    ARRAY_DEL(&ecs->entity_ptrs, entity_idx);
                    entity_idx--;

                    HASHMAP_DEL(&ecs->entity_map, &entity->eid);

                    break;
                }
                default:
                    break;
            }
        }
    }
}

EntityID ecs_entity_create(EcsHandle *ecs, const EntityCreateInfo *info) {
    EntityID new_eid = {
        .guid = ++ecs->entity_counter,
    };

    u64 vars_size = info->var_types.len * sizeof(EntityVar);
    Entity new_entity = {
        .eid = new_eid,
        .prev_tick = 0,
        .name = info->name,
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
    HASHMAP_PUT(&ecs->entity_map, &new_eid, &new_entity);

    while (ecs->entity_ptrs.len >= ecs->entity_ptrs.cap) {
        i64 new_cap = 2 * ecs->entity_ptrs.cap;
        u64 elem_size = sizeof(ecs->entity_ptrs.data[0]);
        u64 new_size = new_cap * elem_size;

        EntityPtrs new_entity_ptrs = {
            .arena = arena_make((i64) new_size),
            .len = ecs->entity_ptrs.len,
            .cap = new_cap,
        };

        ARRAY_MAKE(&new_entity_ptrs);
        memcpy(new_entity_ptrs.data, ecs->entity_ptrs.data, ecs->entity_ptrs.len * elem_size);

        arena_free(ecs->entity_ptrs.arena);

        ecs->entity_ptrs = new_entity_ptrs;
    }

    Entity *new_entity_ptr = &(HASHMAP_GET(&ecs->entity_map, &new_eid)->value);

    i64 new_entity_ptr_idx = 0;

    for (i64 entity_ptr_idx = 0; entity_ptr_idx < ecs->entity_ptrs.len; entity_ptr_idx++) {
        Entity *entity_ptr = *ARRAY_GET(&ecs->entity_ptrs, entity_ptr_idx);
        if ((entity_ptr->priority) > (new_entity_ptr->priority)) {
            break;
        }

        new_entity_ptr_idx = entity_ptr_idx + 1;
    }

    ARRAY_PUT(&ecs->entity_ptrs, new_entity_ptr_idx, &new_entity_ptr);

    return new_eid;
}

bool ecs_entity_exists(EcsHandle *ecs, EntityID eid) {
    return HASHMAP_GET(&ecs->entity_map, &eid);
}

static void *get_entity_var(EcsHandle *ecs, EntityID eid, i64 var_idx, EntityVarType var_type) {
    void *var_ptr = nullptr;

    auto entity_pair = HASHMAP_GET(&ecs->entity_map, &eid);
    if (entity_pair) {
        Entity *entity = &entity_pair->value;

        EntityVar *var_elem = ARRAY_GET(&entity->vars, var_idx);

        if (var_type == var_elem->type) {
            switch (var_type) {
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

i64 *ecs_entity_var_get_i64(EcsHandle *ecs, EntityID eid, i64 var_idx) {
    return get_entity_var(ecs, eid, var_idx, ENTITY_VAR_TYPE_I64);
}

u64 *ecs_entity_var_get_u64(EcsHandle *ecs, EntityID eid, i64 var_idx) {
    return get_entity_var(ecs, eid, var_idx, ENTITY_VAR_TYPE_U64);
}

f64 *ecs_entity_var_get_f64(EcsHandle *ecs, EntityID eid, i64 var_idx) {
    return get_entity_var(ecs, eid, var_idx, ENTITY_VAR_TYPE_F64);
}

void **ecs_entity_var_get_ptr(EcsHandle *ecs, EntityID eid, i64 var_idx) {
    return get_entity_var(ecs, eid, var_idx, ENTITY_VAR_TYPE_PTR);
}

EntityID *ecs_entity_var_get_eid(EcsHandle *ecs, EntityID eid, i64 var_idx) {
    return get_entity_var(ecs, eid, var_idx, ENTITY_VAR_TYPE_EID);
}

void ecs_entity_component_add(EcsHandle *ecs, EntityID eid, i64 component_idx) {
    if (ecs_entity_component_has(ecs, eid, component_idx)) {
        return;
    }

    Entity *entity = &HASHMAP_GET(&ecs->entity_map, &eid)->value;
    if (!entity->component_flags.data) {
        entity->component_flags = bits_make(nullptr, ecs->component_arrays.len);
    }

    ComponentArray *component_array = ARRAY_GET(&ecs->component_arrays, component_idx);
    component_array_put(component_array, &eid);

    bits_set(&entity->component_flags, component_idx);
}

void ecs_entity_component_del(EcsHandle *ecs, EntityID eid, i64 component_idx) {
    if (!ecs_entity_component_has(ecs, eid, component_idx)) {
        return;
    }

    Entity *entity = &HASHMAP_GET(&ecs->entity_map, &eid)->value;
    if (!entity->component_flags.data) {
        crash_msg("Tried unsetting component flag %d from uninitialised bit field\n", component_idx);
    }

    ComponentArray *component_array = ARRAY_GET(&ecs->component_arrays, component_idx);
    component_array_del(component_array, &eid);

    bits_unset(&entity->component_flags, component_idx);
}

bool ecs_entity_component_has(EcsHandle *ecs, EntityID eid, i64 component_idx) {
    if (component_idx < 0 || component_idx >= ecs->component_arrays.len) {
        crash_msg("Component index %d exceeds component array length of %d\n", component_idx,
                  ecs->component_arrays.len);
    }

    if (!ecs_entity_exists(ecs, eid)) {
        crash_msg("Entity %u does not exist\n", eid);
    }

    Entity *entity = &HASHMAP_GET(&ecs->entity_map, &eid)->value;
    if (!entity->component_flags.data) {
        return false;
    }

    return bits_is_set(&entity->component_flags, component_idx);
}
