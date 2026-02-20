//
// Created by wright on 2/20/26.
//

#ifndef ALTCORE_HASHMAP_H
#define ALTCORE_HASHMAP_H

#include "libs/stb_ds.h"
#include <string.h>

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

#ifndef HASHMAP_TYPE
#define HASHMAP_TYPE(key_type, val_type) \
    HashmapDelFreq del_freq; \
    struct { \
        key_type key; \
        val_type value; \
    } *hash; \
    HashmapType type;
#endif

#ifndef HASHMAP_TYPE_TO_STR
#define HASHMAP_TYPE_TO_STR(type) #type
#endif

void hashmap_detect_type(HashmapType *type, const char *key_type_str);

#ifndef HASHMAP_MAKE
#define HASHMAP_MAKE(hashmap_ptr, default_val_ptr) \
    do { \
        hashmap_detect_type((hashmap_ptr)->type, HASHMAP_TYPE_TO_STR(typeof((hashmap_ptr)->hash->key))); \
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
                shdefault(*(default_val_ptr)); \
                break; \
            } \
            case HASHMAP_TYPE_NON_STR_KEY: \
            { \
                hmdefault(*(default_val_ptr)); \
                break; \
            } \
        } \
    } while(0)
#endif

#endif //ALTCORE_HASHMAP_H
