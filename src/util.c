
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

#include "include/util.h"

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
        PANIC("Couldn't realloc from %ld to %ld bytes", mem->sz,
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
    /* Initialize curl */
    CURL* curl = curl_easy_init();
    if (!curl) {
        PANIC("curl_easy_init returned NULL");
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
        PANIC("curl_easy_perform failed for URL \"%s\": %s", url,
              curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(mem.data);
        return NULL;
    }

    /* Fill JSON object parameter with parsed response */
    cJSON* ret = cJSON_Parse(mem.data);
    if (!ret) {
        PANIC("cJSON_Parse returned NULL");
        curl_easy_cleanup(curl);
        free(mem.data);
        return NULL;
    }

    /* Free stuff */
    curl_easy_cleanup(curl);
    free(mem.data);
    return ret;
}

/* Fill Thread struct from id */
static bool fill_thread_from_id(Thread* out, int id) {
    static char url[255] = { '\0' };
    snprintf(url, 255, "https://a.4cdn.org/" BOARD "/thread/%d.json", id);

    cJSON* thread = json_from_url(url);
    if (!thread) {
        PANIC("json_from_url returned NULL");
        return false;
    }

    cJSON* posts = cJSON_GetObjectItemCaseSensitive(thread, "posts");
    if (!posts || !cJSON_IsArray(posts)) {
        PANIC("Can't find \"posts\" array in thread JSON");
        return false;
    }

    cJSON* fp = cJSON_GetArrayItem(posts, 0);
    if (!fp) {
        PANIC("Can't get first post from \"posts\" array in thread JSON");
        return false;
    }

    cJSON* thread_replies  = cJSON_GetObjectItemCaseSensitive(fp, "replies");
    cJSON* thread_images   = cJSON_GetObjectItemCaseSensitive(fp, "images");
    cJSON* thread_title    = cJSON_GetObjectItemCaseSensitive(fp, "sub");
    cJSON* thread_filename = cJSON_GetObjectItemCaseSensitive(fp, "filename");
    cJSON* thread_ext      = cJSON_GetObjectItemCaseSensitive(fp, "ext");

    if (!thread_replies || !thread_images || !thread_filename || !thread_ext) {
        PANIC("One of the required JSON thread items was NULL");

        printf("%s\n", cJSON_Print(thread));
        return false;
    }

    if (!cJSON_IsNumber(thread_replies) || !cJSON_IsNumber(thread_images) ||
        !cJSON_IsString(thread_filename) || !cJSON_IsString(thread_ext)) {
        PANIC("One of the required JSON thread items had incorrect type");
        return false;
    }

    /* Not all threads have title */
    if (thread_title && cJSON_IsString(thread_title)) {
        char* title = malloc(strlen(thread_title->valuestring));
        strcpy(title, thread_title->valuestring);
        out->title = title;
    } else {
        /* No title, leave NULL */
        out->title = NULL;
    }

    char* filename = malloc(strlen(thread_filename->valuestring) +
                            strlen(thread_ext->valuestring));
    sprintf(filename, "%s%s", thread_filename->valuestring,
            thread_ext->valuestring);

    out->id       = id;
    out->replies  = thread_replies->valueint;
    out->images   = thread_images->valueint;
    out->filename = filename;

    cJSON_Delete(thread);
    return true;
}

/* Fill Thread list from threads.json object */
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

            const uint32_t id = cur_thread_no->valueint;
            if (!fill_thread_from_id(&out[i++], id)) {
                PANIC("fill_thread_from_id returned false");
                return false;
            }

            if (i >= MAX_THREADS) {
                PANIC("Reached MAX_THREADS");
                return false;
            }
        }

        /* FIXME: More than 1 page */
        break;
    }

    return true;
}

/* Free all the allocated strings by fill_thread_from_id() in a Thread array */
void threads_free(Thread* arr) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (arr[i].title)
            free(arr[i].title);
        if (arr[i].filename)
            free(arr[i].filename);
    }
}
