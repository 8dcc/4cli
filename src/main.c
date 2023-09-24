
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

#include "include/util.h"

/* Globals */
CURL* curl = NULL;

int main() {
    /* Initialize curl */
    curl = curl_easy_init();
    if (!curl) {
        PANIC("curl_easy_init returned NULL");
        return 1;
    }

    cJSON* json_threads = json_from_url(THREADS_URL);
    if (!json_threads) {
        PANIC("Couldn't get JSON for threads URL");
        cJSON_Delete(json_threads);
        return 1;
    }

    Thread thread_arr[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++)
        thread_arr[i] = 0;

    if (!threads_from_json(thread_arr, json_threads)) {
        PANIC("Couldn't get thread ID array from json");
        cJSON_Delete(json_threads);
        return 1;
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_arr[i] == 0)
            break;

        if (!print_thread_info(thread_arr[i])) {
            PANIC("Couldn't print thread information");
            cJSON_Delete(json_threads);
            return 1;
        }
    }

    /* Free stuff */
    curl_easy_cleanup(curl);
    cJSON_Delete(json_threads);
    return 0;
}
