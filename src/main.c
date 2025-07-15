
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "include/main.h"
#include "include/util.h"
#include "include/request.h"
#include "include/thread.h"
#include "include/pretty.h"

int main(void) {
    int exit_code = EXIT_SUCCESS;

    /* Initialize curl */
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        PANIC("Failed to initialize 'CURL' object.");
        exit_code = EXIT_FAILURE;
        goto cleanup_curl;
    }

    cJSON* threads_json = request_json_from_url(curl, THREADS_URL);
    if (threads_json == NULL) {
        exit_code = EXIT_FAILURE;
        goto cleanup_curl;
    }

    ThreadId thread_arr[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++)
        thread_arr[i] = 0;

    if (!threads_from_json(thread_arr, threads_json)) {
        PANIC("Couldn't get thread ID array from JSON.");
        exit_code = EXIT_FAILURE;
        goto cleanup_json;
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_arr[i] == 0)
            break;

        if (!print_thread_info(curl, thread_arr[i])) {
            PANIC("Couldn't print thread information.");
            exit_code = EXIT_FAILURE;
            goto cleanup_json;
        }
    }

cleanup_json:
    cJSON_Delete(threads_json);

cleanup_curl:
    curl_easy_cleanup(curl);

    return exit_code;
}
