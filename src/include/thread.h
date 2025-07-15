
#ifndef THREAD_H_
#define THREAD_H_ 1

#include <stddef.h>

#include <cjson/cJSON.h>

typedef unsigned long ThreadId;

/*
 * Fill a list of thread IDs (of the specified maximum size) by parsing the
 * contents of the 'src' JSON.
 */
size_t thread_ids_from_json(ThreadId* dst, size_t dst_sz, cJSON* src);

#endif /* THREAD_H_ */
