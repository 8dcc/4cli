
#ifndef THREAD_H_
#define THREAD_H_ 1

#include <stddef.h>
#include <stdbool.h>

#include <cjson/cJSON.h>

typedef unsigned long ThreadId;

#define MAX_THREADS 255

/*
 * Fill Thread list from threads.json object.
 */
bool threads_from_json(ThreadId* out, cJSON* in);

#endif /* THREAD_H_ */
