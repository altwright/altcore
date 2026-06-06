//
// Created by wright on 6/5/26.
//

#ifndef ALTCORE_ECS_H
#define ALTCORE_ECS_H

#include "types.h"

typedef struct ENTITY_ID_T {
    i64 guid;
} EntityID;

typedef enum ENTITY_VAR_TYPE_E {
#ifndef X_ENTITY_VAR_TYPES
#define X_ENTITY_VAR_TYPES \
    X(I32) \
    X(U32) \
    X(F32) \
    X(I64) \
    X(U64) \
    X(F64) \
    X(PTR) \
    X(EID) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    ENTITY_VAR_TYPE_##type,
#endif
    X_ENTITY_VAR_TYPES
#undef X
} EntityVarType;

typedef struct ENTITY_VAR_T {
    EntityVarType type;

    union {
        i32 i32_val;
        u32 u32_val;
        f32 f32_val;
        i64 i64_val;
        u64 u64_val;
        f64 f64_val;
        void *ptr_val;
        EntityID eid_val;
    } data;
} EntityVar;

typedef enum ENTITY_COMPONENT_INDEX_E : i64 {
#ifndef X_ENTITY_COMPONENT_INDEXES
#define X_ENTITY_COMPONENT_INDEXES \
    X(POSITION) \
    X(ROTATION) \
    X(SCALE) \
    X(COUNT)
#endif
#ifndef X
#define X(component) \
    ENTITY_COMPONENT_INDEX_##component,
#endif
    X_ENTITY_COMPONENT_INDEXES
#undef X
} EntityComponentIndex;

typedef enum ENTITY_COMPONENT_FLAG_E : u64 {
#ifndef X
#define X(component) \
    ENTITY_COMPONENT_FLAG_##component = 1ULL << ENTITY_COMPONENT_INDEX_##component,
#endif
    X_ENTITY_COMPONENT_INDEXES
#undef X
} EntityComponentFlag;

typedef u64 EntityComponentFlags;

typedef enum ENTITY_TICK_RETURN_CODE_E {
#ifndef X_ENTITY_TICK_RETURN_CODES
#define X_ENTITY_TICK_RETURN_CODES \
    X(RUNNING) \
    X(EXIT)
#endif
#ifndef X
#define X(code) \
    ENTITY_TICK_RETURN_CODE_##code,
#endif
    X_ENTITY_TICK_RETURN_CODES
#undef X
} EntityTickReturnCode;

typedef EntityTickReturnCode (*EntityTickFnPtr)(EntityID eid);

typedef struct ENTITY_CREATE_INFO_T {
    const char *name;

    EntityComponentFlags components;

    struct {
        EntityVar *data;
        i32 len;
    } vars;

    u64 priority;

    i32 tick_fn_ptr_idx;
} EntityCreateInfo;

void ecs_init();

void ecs_deinit();

void ecs_set_tick_fn_ptrs(EntityTickFnPtr *tick_fn_ptrs, i32 tick_fn_ptrs_len);

void ecs_tick();

EntityID ecs_create_entity(const EntityCreateInfo *info);

i32 *ecs_get_i32_var(EntityID eid, i32 var_idx);

u32 *ecs_get_u32_var(EntityID eid, i32 var_idx);

f32 *ecs_get_f32_var(EntityID eid, i32 var_idx);

i64 *ecs_get_i64_var(EntityID eid, i32 var_idx);

u64 *ecs_get_u64_var(EntityID eid, i32 var_idx);

f64 *ecs_get_f64_var(EntityID eid, i32 var_idx);

void **ecs_get_ptr_var(EntityID eid, i32 var_idx);

EntityID *ecs_get_eid_var(EntityID eid, i32 var_idx);

EntityComponentFlags ecs_get_components(EntityID eid);

i64 ecs_get_priority(EntityID eid);

void ecs_set_priority(EntityID eid, u64 priority);

f32x3 *ecs_get_entity_position(EntityID eid);

f32x4 *ecs_get_entity_rotation(EntityID eid);

f32x3 *ecs_get_entity_scale(EntityID eid);

i32 ecs_get_positions(f32x3 **positions, EntityID **eids, i32 *len);

i32 ecs_get_rotations(f32x4 **rotations, EntityID **eids, i32 *len);

i32 ecs_get_scales(f32x3 **scales, EntityID **eids, i32 *len);

#endif //ALTCORE_ECS_H
