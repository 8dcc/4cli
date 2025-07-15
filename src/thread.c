
#include <cjson/cJSON.h>

#include "include/thread.h"
#include "include/util.h"

bool threads_from_json(ThreadId* out, cJSON* in) {
    int i = 0;

    cJSON* cur_page;
    cJSON_ArrayForEach(cur_page, in) {
        cJSON* json_threads =
          cJSON_GetObjectItemCaseSensitive(cur_page, "threads");
        if (!json_threads) {
            PANIC("Couldn't get thread list cJSON object from page");
            return false;
        }

        cJSON* cur_thread;
        cJSON_ArrayForEach(cur_thread, json_threads) {
            cJSON* cur_thread_no =
              cJSON_GetObjectItemCaseSensitive(cur_thread, "no");
            if (!cJSON_IsNumber(cur_thread_no)) {
                PANIC("Thread number was not an integer");
                return false;
            }

            out[i++] = cur_thread_no->valueint;

            if (i >= MAX_THREADS) {
                PANIC("Reached MAX_THREADS");
                return false;
            }
        }
    }

    return true;
}
