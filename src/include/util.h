#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "../dependencies/cJSON/cJSON.h"

#include "color.h"

#define BOARD       "g"
#define THREADS_URL "https://a.4cdn.org/" BOARD "/threads.json"

#define MAX_THREADS 255 /* Thread[] */

#define PANIC(...)                                                     \
    {                                                                  \
        fprintf(stderr, COL_ERROR "[panic] " COL_WARN "%s: " COL_NORM, \
                __func__);                                             \
        fprintf(stderr, __VA_ARGS__);                                  \
        fputc('\n', stderr);                                           \
    }

/* Thread number */
typedef uint32_t Thread;

/* src/main.c */
extern CURL* curl;

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url);

/* Fill Thread list from threads.json object */
bool threads_from_json(Thread* out, cJSON* in);

/* Print thread information */
bool print_thread_info(Thread id);

#endif /* UTIL_H_ */
