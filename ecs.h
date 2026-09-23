//
// Created by wright on 6/5/26.
//

#ifndef ALTCORE_ECS_H
#define ALTCORE_ECS_H

#include "types.h"
#include "draw/lights.h"

typedef struct ECS_HANDLE_T EcsHandle;

typedef struct ENTITY_ID_T {
    u64 guid;
} EntityID;

typedef enum ENTITY_VAR_TYPE_E {
#define X_ENTITY_VAR_TYPES \
    X(I64) \
    X(U64) \
    X(F64) \
    X(PTR) \
    X(EID) \
    X(COUNT)
#define X(type) \
    ENTITY_VAR_TYPE_##type,
    X_ENTITY_VAR_TYPES
#undef X
} EntityVarType;

typedef enum ENTITY_COMPONENT_TYPE_E {
#define X_ENTITY_COMPONENT_TYPES \
    X(I8) \
    X(U8) \
    X(I16) \
    X(U16) \
    X(I32) \
    X(U32) \
    X(I64) \
    X(U64) \
    X(F32) \
    X(F64) \
    X(I8X2) \
    X(U8X2) \
    X(I16X2) \
    X(U16X2) \
    X(I32X2) \
    X(U32X2) \
    X(I64X2) \
    X(U64X2) \
    X(F32X2) \
    X(F64X2) \
    X(I8X3) \
    X(U8X3) \
    X(I16X3) \
    X(U16X3) \
    X(I32X3) \
    X(U32X3) \
    X(I64X3) \
    X(U64X3) \
    X(F32X3) \
    X(F64X3) \
    X(I8X4) \
    X(U8X4) \
    X(I16X4) \
    X(U16X4) \
    X(I32X4) \
    X(U32X4) \
    X(I64X4) \
    X(U64X4) \
    X(F32X4) \
    X(F64X4)
#define X(type) \
    ENTITY_COMPONENT_TYPE_##type,
    X_ENTITY_COMPONENT_TYPES
#undef X
    ENTITY_COMPONENT_TYPE_COUNT
} EntityComponentType;

typedef struct ENTITY_VAR_T {
    EntityVarType type;

    union {
        i64 i64_val;
        u64 u64_val;
        f64 f64_val;
        void *ptr_val;
        EntityID eid_val;
    } data;
} EntityVar;

typedef enum ENTITY_TICK_RETURN_CODE_E {
#define X_ENTITY_TICK_RETURN_CODES \
    X(RUNNING) \
    X(EXIT)
#define X(code) \
    ENTITY_TICK_RETURN_CODE_##code,
    X_ENTITY_TICK_RETURN_CODES
#undef X
} EntityTickReturnCode;

typedef EntityTickReturnCode (*EntityTickFnPtr)(EcsHandle *ecs, EntityID eid);

typedef void (*EntitySerializeFnPtr)(EcsHandle *ecs, EntityID eid, i32 var_idx, u8 *out_serialized_var,
                                     i64 *out_serialized_var_len);

typedef void (*EntityDeserializeFnPtr)(EcsHandle *ecs, EntityID eid, i32 var_idx, u8 *in_serialized_var,
                                       i64 in_serialized_var_len);

typedef struct ENTITY_FN_PTRS_T {
    EntityTickFnPtr tick_fn_ptr;
    EntitySerializeFnPtr serial_fn_ptr;
    EntityDeserializeFnPtr deserial_fn_ptr;
} EntityFnPtrs;

typedef struct ENTITY_CREATE_INFO_T {
    const char *name;

    struct {
        EntityVarType *data;
        i64 len;
    } var_types;

    u64 priority;

    i64 entity_type_idx;
} EntityCreateInfo;

typedef struct ECS_CREATE_INFO_T {
    const struct {
        EntityComponentType *data;
        i64 len;
    } component_types;

    f64 tick_len_s;
} EcsCreateInfo;

EcsHandle *ecs_create(const EcsCreateInfo *info);

void ecs_destroy(EcsHandle *ecs);

void ecs_set_entity_fn_ptrs(EcsHandle *ecs, EntityFnPtrs *fn_ptrs, i64 fn_ptrs_len);

void ecs_tick(EcsHandle *ecs);

EntityID ecs_entity_create(EcsHandle *ecs, const EntityCreateInfo *info);

bool ecs_entity_exists(EcsHandle *ecs, EntityID eid);

i64 *ecs_entity_var_get_i64(EcsHandle *ecs, EntityID eid, i64 var_idx);

u64 *ecs_entity_var_get_u64(EcsHandle *ecs, EntityID eid, i64 var_idx);

f64 *ecs_entity_var_get_f64(EcsHandle *ecs, EntityID eid, i64 var_idx);

void **ecs_entity_var_get_ptr(EcsHandle *ecs, EntityID eid, i64 var_idx);

EntityID *ecs_entity_var_get_eid(EcsHandle *ecs, EntityID eid, i64 var_idx);

void ecs_entity_component_add(EcsHandle *ecs, EntityID eid, i64 component_idx);

void ecs_entity_component_del(EcsHandle *ecs, EntityID eid, i64 component_idx);

bool ecs_entity_component_has(EcsHandle *ecs, EntityID eid, i64 component_idx);

i32 *ecs_entity_component_get_i32(EcsHandle *ecs, EntityID eid, i64 component_idx);

#endif //ALTCORE_ECS_H
