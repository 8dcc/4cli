
#ifndef UTIL_H_
#define UTIL_H_ 1

#include <stdio.h>  /* fprintf, stderr, etc. */
#include <string.h> /* memmove */

#include "color.h"

/*
 * Return the length of an array at compile-time.
 */
#define ARRLEN(ARR) (sizeof(ARR) / sizeof((ARR)[0]))

/*
 * Return the length of a string literal at compile-time.
 */
#define STRLEN(STR) (ARRLEN(STR) - 1)

/*
 * Move a string to a destination address, allowing buffer overlapping.
 */
#define STRMOVE(DST, STR) memmove(DST, STR, strlen(STR) + 1)

/*
 * Print an error and a newline.
 */
#define ERR(...)                                                               \
    do {                                                                       \
        fprintf(stderr, COL_ERROR "4cli: " COL_WARN);                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, COL_NORM "\n");                                        \
    } while (0)

#endif /* UTIL_H_ */
