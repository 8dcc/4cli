
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

#include "include/thread.h"
#include "include/util.h"
#include "include/main.h"

typedef struct {
    char* data;
    size_t sz;
} MemChunk;

/* Curl callback */
static size_t parse_curl_response(char* response,
                                  size_t item_sz,
                                  size_t item_num,
                                  void* user_data) {
    /* Total bytes. This value should be returned */
    size_t real_sz = item_sz * item_num;

    /* Set by CURLOPT_WRITEDATA in main */
    MemChunk* mem = (MemChunk*)user_data;

    char* ptr = realloc(mem->data, mem->sz + real_sz + 1);
    if (!ptr) {
        PANIC("Couldn't realloc from %ld to %ld bytes",
              mem->sz,
              mem->sz + real_sz + 1);
        return 0;
    }

    mem->data = ptr;
    memcpy(&mem->data[mem->sz], response, real_sz);
    mem->sz += real_sz;
    mem->data[mem->sz] = '\0';

    return real_sz;
}

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url) {
    /* Main memory chunk used for curl responses. The data attribute will get
     * reallocated in parse_curl_response() */
    MemChunk mem = {
        .data = malloc(1),
        .sz   = 0,
    };

    /* Set target URL, callback function and user_data parameter for callback */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_curl_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);

    /* Make request to get the JSON string */
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        PANIC("curl_easy_perform failed for URL \"%s\": %s",
              url,
              curl_easy_strerror(res));
        free(mem.data);
        return NULL;
    }

    /* Fill JSON object parameter with parsed response */
    cJSON* ret = cJSON_Parse(mem.data);
    if (!ret) {
        PANIC("cJSON_Parse returned NULL");
        free(mem.data);
        return NULL;
    }

    free(mem.data);
    return ret;
}

/* Fill thread ID list from threads.json object */
bool threads_from_json(Thread* out, cJSON* in) {
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
