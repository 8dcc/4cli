
#include <stddef.h>
#include <string.h> /* memcpy */
#include <stdlib.h> /* realloc */

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "include/request.h"
#include "include/util.h"
#include "include/main.h"

typedef struct {
    char* data;
    size_t sz;
} Buffer;

/*
 * Callback used as 'CURLOPT_WRITEFUNCTION', which will be called whenever some
 * data is received.
 */
static size_t data_received_callback(char* response, size_t item_sz,
                                     size_t item_num, void* user_data) {
    /*
     * Total received bytes. This is the value that this callback should return
     * on success.
     */
    const size_t real_sz = item_sz * item_num;

    /*
     * The 'user_data' argument will contain the value we set through
     * 'CURLOPT_WRITEDATA'.
     */
    Buffer* buffer = (Buffer*)user_data;

    char* ptr = realloc(buffer->data, buffer->sz + real_sz + 1);
    if (ptr == NULL) {
        PANIC("Couldn't realloc from %ld to %ld bytes.",
              buffer->sz,
              buffer->sz + real_sz + 1);
        return 0;
    }

    buffer->data = ptr;
    memcpy(&buffer->data[buffer->sz], response, real_sz);
    buffer->sz += real_sz;
    buffer->data[buffer->sz] = '\0';

    return real_sz;
}

cJSON* request_json_from_url(CURL* curl, const char* url) {
    cJSON* result = NULL;

    /*
     * Main buffer used to store curl responses. The 'data' member will get
     * reallocated as needed in 'on_curl_write'.
     */
    Buffer buffer = {
        .data = NULL,
        .sz   = 0,
    };

    /*
     * Set target URL, the callback function and the 'user_data' parameter of
     * the callback.
     */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_received_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    /* Make request to get the JSON string */
    const CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
        PANIC("Failed to perform request to '%s': %s",
              url,
              curl_easy_strerror(code));
        goto done;
    }

    /* Make sure the callback filled the buffer with some data */
    if (buffer.data == NULL || buffer.sz <= 0) {
        PANIC("Received an empty response buffer.");
        goto done;
    }

    /* Fill JSON object parameter with parsed response */
    result = cJSON_Parse(buffer.data);
    if (result == NULL) {
        PANIC("Could not parse response as JSON.");
        goto done;
    }

done:
    /*
     * Free the data that might have been allocated inside
     * 'data_received_callback'.
     */
    free(buffer.data);
    return result;
}
