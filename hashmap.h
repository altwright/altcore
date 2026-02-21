//
// Created by wright on 2/20/26.
//

#ifndef ALTCORE_HASHMAP_H
#define ALTCORE_HASHMAP_H

#include "libs/stb_ds.h"
#include "types.h"

typedef enum HASHMAP_TYPE_E {
#ifndef X_HASHMAP_TYPES
#define X_HASHMAP_TYPES \
    X(NON_STR_KEY) \
    X(STR_KEY) \
    X(COUNT)
#endif

#ifndef X
#define X(type) \
    HASHMAP_TYPE_##type,
#endif
    X_HASHMAP_TYPES
#undef X
} HashmapType;

typedef enum HASHMAP_DEL_FREQ_E {
#ifndef X_HASHMAP_DEL_FREQS
#define X_HASHMAP_DEL_FREQS \
    X(LOW) \
    X(HIGH) \
    X(COUNT)
#endif

#ifndef X
#define X(freq) \
    HASHMAP_DEL_FREQ_##freq,
#endif
    X_HASHMAP_DEL_FREQS
#undef X
} HashmapDelFreq;

#ifndef HASHMAP_FIELDS
#define HASHMAP_FIELDS(key_type, val_type) \
    HashmapType type; \
    HashmapDelFreq del_freq; \
    struct { \
        key_type key; \
        val_type value; \
    } *hash;
#endif

#ifndef HASHMAP_MAKE
#define HASHMAP_MAKE(hashmap_ptr, default_val_ptr) \
    do { \
        switch ((hashmap_ptr)->type) { \
            case HASHMAP_TYPE_STR_KEY: \
            { \
                switch ((hashmap_ptr)->del_freq) \
                { \
                    case HASHMAP_DEL_FREQ_LOW: \
                    { \
                        sh_new_arena((hashmap_ptr)->hash); \
                        break; \
                    } \
                    case HASHMAP_DEL_FREQ_HIGH: \
                    { \
                        sh_new_strdup((hashmap_ptr)->hash); \
                        break; \
                    } \
                    default: \
                        break; \
                } \
                shdefault((hashmap_ptr)->hash, *(default_val_ptr)); \
                break; \
            } \
            case HASHMAP_TYPE_NON_STR_KEY: \
            { \
                hmdefault((hashmap_ptr)->hash, *(default_val_ptr)); \
                break; \
            } \
            default: \
                break; \
        } \
    } while(0)
#endif

#ifndef HASHMAP_FREE
#define HASHMAP_FREE(hashmap_ptr) \
    do { \
        switch ((hashmap_ptr)->type) { \
            case HASHMAP_TYPE_STR_KEY: \
            { \
                shfree((hashmap_ptr)->hash); \
                break; \
            } \
            case HASHMAP_TYPE_NON_STR_KEY: \
            { \
                hmfree((hashmap_ptr)->hash); \
                (hashmap_ptr)->hash = nullptr; \
                break; \
            } \
            default: \
                break; \
        } \
    } while(0)
#endif

#ifndef HASHMAP_PUT
#define HASHMAP_PUT(hashmap_ptr, key_ptr, val_ptr) \
    do { \
        switch ((hashmap_ptr)->type) { \
            case HASHMAP_TYPE_STR_KEY: { \
                shput((hashmap_ptr)->hash, *((char**)(key_ptr)), *(val_ptr)); \
                break; \
            } \
            case HASHMAP_TYPE_NON_STR_KEY: { \
                hmput((hashmap_ptr)->hash, *(key_ptr), *(val_ptr)); \
                break; \
            } \
            default: \
                break; \
        } \
    } while(0)
#endif

#ifndef HASHMAP_GET
#define HASHMAP_GET(hashmap_ptr, key_ptr) \
    ( \
        (hashmap_ptr)->type == HASHMAP_TYPE_STR_KEY ? \
            shget((hashmap_ptr)->hash, *(key_ptr)) : \
            ( \
                (hashmap_ptr)->type == HASHMAP_TYPE_NON_STR_KEY ? \
                    hmget((hashmap_ptr)->hash, *(key_ptr)) : \
                    *((typeof((hashmap_ptr)->hash->value)*) kNullPtr) \
            ) \
    )
#endif

#endif //ALTCORE_HASHMAP_H
