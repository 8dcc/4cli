
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
            PANIC("Couldn't get thread list cJSON object from page");
            return 0;
        }

        /* Iterate each thread in the current page */
        cJSON* cur_thread;
        cJSON_ArrayForEach(cur_thread, threads) {
            /* Obtain the thread number of the current element */
            cJSON* cur_thread_no =
              cJSON_GetObjectItemCaseSensitive(cur_thread, "no");
            if (!cJSON_IsNumber(cur_thread_no)) {
                PANIC("Thread number is not an integer.");
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
