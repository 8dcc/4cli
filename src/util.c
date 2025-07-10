
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "include/util.h"

/* string.h function does weird stuff when strings overlap (our case) */
char* my_strcpy(char* dest, const char* src) {
    size_t i;

    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];

    dest[i] = '\0';

    return dest;
}

bool is_cjson_int(cJSON* p) {
    return p && cJSON_IsNumber(p);
}

bool is_cjson_str(cJSON* p) {
    return p && cJSON_IsString(p);
}
