
#ifndef THREAD_H_
#define THREAD_H_ 1

#include <stdint.h>
#include <stdbool.h>

#include "../dependencies/cJSON/cJSON.h"

/* Thread number */
typedef uint32_t Thread;

#define MAX_THREADS 255 /* Thread[] */

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url);

/* Fill Thread list from threads.json object */
bool threads_from_json(Thread* out, cJSON* in);

#endif /* THREAD_H_ */
