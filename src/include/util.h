#ifndef UTIL_H_
#define UTIL_H_

#include "../dependencies/cJSON/cJSON.h"

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url);

#endif /* UTIL_H_ */
