
#ifndef UTIL_H_
#define UTIL_H_ 1

#include <stdbool.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "color.h"

/*
 * Return the length of an array at compile-time.
 */
#define ARRLEN(ARR) (sizeof(ARR) / sizeof((ARR)[0]))

/*
 * Return the length of a string literal at compile-time.
 */
#define STRLEN(STR) (ARRLEN(STR) - 1)

#define STRMOVE(DST, STR) memmove(DST, STR, strlen(STR) + 1)

#define ERR(...)                                                               \
    do {                                                                       \
        fprintf(stderr, COL_ERROR "4cli: " COL_WARN);                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, COL_NORM "\n");                                        \
    } while (0)

bool is_cjson_int(cJSON* p);

bool is_cjson_str(cJSON* p);

#endif /* UTIL_H_ */
