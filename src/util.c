
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "include/util.h"

bool is_cjson_int(cJSON* p) {
    return p && cJSON_IsNumber(p);
}

bool is_cjson_str(cJSON* p) {
    return p && cJSON_IsString(p);
}
