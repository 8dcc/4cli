/*
 * Copyright 2025 8dcc
 *
 * This file is part of 4cli.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

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
