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

#include <cjson/cJSON.h>

#include "include/thread.h"
#include "include/util.h"

size_t thread_ids_from_json(ThreadId* dst, size_t dst_sz, cJSON* src) {
    size_t written = 0;

    /* Iterate each page in the array */
    cJSON* cur_page;
    cJSON_ArrayForEach(cur_page, src) {
        /* Obtain the "threads" array of the current page */
        cJSON* threads = cJSON_GetObjectItemCaseSensitive(cur_page, "threads");
        if (!threads) {
            ERR("Couldn't get thread list cJSON object from page.");
            return 0;
        }

        /* Iterate each thread in the current page */
        cJSON* cur_thread;
        cJSON_ArrayForEach(cur_thread, threads) {
            /* Obtain the thread number of the current element */
            cJSON* cur_thread_no =
              cJSON_GetObjectItemCaseSensitive(cur_thread, "no");
            if (!cJSON_IsNumber(cur_thread_no)) {
                ERR("Thread number is not an integer.");
                return 0;
            }

            dst[written++] = cur_thread_no->valueint;

            /* Did we reach the end if the destination list? */
            if (written > dst_sz)
                return written;
        }
    }

    return written;
}
