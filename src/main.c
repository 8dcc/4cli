
#include <stdio.h>

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

#define TARGET_URL "https://a.4cdn.org/g/threads.json"

int main(int argc, char** argv) {
    /* Initialize curl */
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: curl_easy_init returned NULL\n");
        return 1;
    }

    /* Set target URL and make request to get the JSON string */
    curl_easy_setopt(curl, CURLOPT_URL, TARGET_URL);
    CURLcode curl_request_code = curl_easy_perform(curl);
    if (curl_request_code != CURLE_OK) {
        fprintf(stderr,
                "Error: curl_easy_perform didn't return CURLE_OK for URL "
                "\"%s\"\n",
                TARGET_URL);
        return 1;
    }

    /* Get JSON object from response */
    cJSON* threads = cJSON_CreateObject();

    /* Free stuff */
    cJSON_Delete(threads);
    curl_easy_cleanup(curl);
    return 0;
}
