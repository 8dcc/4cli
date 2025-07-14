
#ifndef REQUEST_H_
#define REQUEST_H_ 1

#include <cjson/cJSON.h>

/*
 * Request the contents of the specified URL, and parse them as JSON.
 */
cJSON* request_json_from_url(const char* url);

#endif /* REQUEST_H_ */
