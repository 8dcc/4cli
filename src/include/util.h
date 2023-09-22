#ifndef UTIL_H_
#define UTIL_H_

#include "../dependencies/cJSON/cJSON.h"

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url);

/* Get object with threads from threads.json object */
cJSON* get_threads_from_page(cJSON* pages, int page);

#endif /* UTIL_H_ */
