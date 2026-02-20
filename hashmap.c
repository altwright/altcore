//
// Created by wright on 2/20/26.
//

#include "hashmap.h"

#define STB_DS_IMPLEMENTATION
#include "libs/stb_ds.h"

void hashmap_detect_type(HashmapType* type, const char* key_type_str) {
    size_t key_type_str_len = strlen(key_type_str);

    if (strncmp(key_type_str, "char *", key_type_str_len) == 0 ||
        strncmp(key_type_str, "const char *", key_type_str_len) == 0) {
        *type = HASHMAP_TYPE_STR_KEY;
    } else {
        *type = HASHMAP_TYPE_NON_STR_KEY;
    }
}
