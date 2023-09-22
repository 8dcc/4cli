
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dependencies/cJSON/cJSON.h"
#include "include/util.h"

#define TARGET_URL "https://a.4cdn.org/g/threads.json"

int main() {
    cJSON* pages = json_from_url(TARGET_URL);
    if (!pages) {
        fprintf(stderr, "4cli: Couldn't parse URL for pages\n");
        cJSON_Delete(pages);
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        cJSON* page_threads = get_threads_from_page(pages, i);
        if (!page_threads) {
            fprintf(stderr, "4cli: Couldn't get pages[%d].threads\n", i);
            cJSON_Delete(pages);
            return 1;
        }

        printf("\n----[ Page %d ]----\n\n", i);

        char* tmp = cJSON_Print(page_threads);
        printf("%s", tmp);
        free(tmp);
    }

    /* Free stuff */
    cJSON_Delete(pages);
    return 0;
}
