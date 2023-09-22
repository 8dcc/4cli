
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

typedef struct {
    char* data;
    size_t sz;
} MemChunk;

/*----------------------------------------------------------------------------*/

/* Curl callback */
static size_t parse_curl_response(char* response, size_t item_sz,
                                  size_t item_num, void* user_data) {
    /* Total bytes. This value should be returned */
    size_t real_sz = item_sz * item_num;

    /* Set by CURLOPT_WRITEDATA in main */
    MemChunk* mem = (MemChunk*)user_data;

    char* ptr = realloc(mem->data, mem->sz + real_sz + 1);
    if (!ptr) {
        fprintf(stderr,
                "parse_curl_response: Couldn't realloc from %ld to %ld bytes\n",
                mem->sz, mem->sz + real_sz + 1);
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
    /* Initialize curl */
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "json_from_url: curl_easy_init returned NULL\n");
        return NULL;
    }

    /* Main memory chunk used for curl responses. mem.data will get reallocated
     * in parse_curl_response() */
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
        fprintf(stderr,
                "json_from_url: curl_easy_perform failed for URL \"%s\": %s\n",
                url, curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(mem.data);
        return NULL;
    }

    /* Fill JSON object parameter with parsed response */
    cJSON* ret = cJSON_Parse(mem.data);
    if (!ret) {
        fprintf(stderr, "json_from_url: cJSON_Parse returned NULL\n");
        curl_easy_cleanup(curl);
        free(mem.data);
        return NULL;
    }

    /* Free stuff */
    curl_easy_cleanup(curl);
    free(mem.data);
    return ret;
}

/* Get object with threads from threads.json object */
cJSON* get_threads_from_page(cJSON* pages, int page) {
    cJSON* page_json = cJSON_GetArrayItem(pages, page);
    if (!page_json) {
        fprintf(stderr,
                "get_threads_from_page: Couldn't get cJSON object for page "
                "%d\n",
                page);
        return NULL;
    }

    return cJSON_GetObjectItemCaseSensitive(page_json, "threads");
}
