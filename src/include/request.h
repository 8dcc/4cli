
#ifndef REQUEST_H_
#define REQUEST_H_ 1

#include <curl/curl.h>
#include <cjson/cJSON.h>

/*
 * Request the contents of the specified URL, and parse them as JSON. The 'curl'
 * argument should have been initialized through 'curl_easy_init'.
 */
cJSON* request_json_from_url(CURL* curl, const char* url);

#endif /* REQUEST_H_ */
