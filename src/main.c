
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

    cJSON* root_json = request_json_from_url(curl, THREADS_URL);
    if (root_json == NULL) {
        exit_code = EXIT_FAILURE;
        goto cleanup_curl;
    }

    static ThreadId thread_ids[MAX_THREADS] = { 0 };
    const size_t retreived_ids =
      thread_ids_from_json(thread_ids, ARRLEN(thread_ids), root_json);
    if (retreived_ids <= 0) {
        exit_code = EXIT_FAILURE;
        goto cleanup_json;
    }

    for (size_t i = 0; i < retreived_ids; i++) {
        const ThreadId cur_thread_id = thread_ids[i];
        if (cur_thread_id == 0)
            continue;

        static char cur_thread_url[255] = { '\0' };
        if (snprintf(cur_thread_url,
                     sizeof(cur_thread_url),
                     "https://a.4cdn.org/" BOARD "/thread/%lu.json",
                     cur_thread_id) < 0)
            continue;

        cJSON* cur_thread = request_json_from_url(curl, cur_thread_url);
        if (cur_thread == NULL)
            continue;

        if (!pretty_print_thread(cur_thread))
            PANIC("Could not print contents of thread with ID %lu",
                  cur_thread_id);

        cJSON_Delete(cur_thread);
    }

cleanup_json:
    cJSON_Delete(root_json);

cleanup_curl:
    curl_easy_cleanup(curl);

    return exit_code;
}
