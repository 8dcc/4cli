
#ifndef THREAD_H_
#define THREAD_H_ 1

#include <stdint.h>
#include <stdbool.h>

#include <cjson/cJSON.h>

/* Thread number */
typedef uint32_t Thread;

#define MAX_THREADS 255 /* Thread[] */

/*
 * Fill Thread list from threads.json object.
 */
bool threads_from_json(Thread* out, cJSON* in);

#endif /* THREAD_H_ */
