
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dependencies/cJSON/cJSON.h"
#include "include/util.h"

#define TARGET_URL "https://a.4cdn.org/g/threads.json"

int main() {
    cJSON* threads = json_from_url(TARGET_URL);
    if (!threads) {
        fprintf(stderr, "4cli: Could not parse URL\n");
        cJSON_Delete(threads);
        return 1;
    }

    char* tmp = cJSON_Print(threads);
    printf("%s", tmp);
    free(tmp);

    /* Free stuff */
    cJSON_Delete(threads);
    return 0;
}
