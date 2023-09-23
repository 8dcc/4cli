#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include "../dependencies/cJSON/cJSON.h"

#define BOARD       "g"
#define THREADS_URL "https://a.4cdn.org/" BOARD "/threads.json"

#define MAX_THREADS 255 /* Thread[] */

#define PANIC(...)                                 \
    {                                              \
        fprintf(stderr, "[panic] %s: ", __func__); \
        fprintf(stderr, __VA_ARGS__);              \
        fputc('\n', stderr);                       \
    }

typedef struct {
    uint32_t id;      /* no */
    uint32_t replies; /* replies */
    uint32_t images;  /* images */
    char* title;      /* sub */
    char* filename;   /* filename + ext */
} Thread;

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url);

/* Fill Thread list from threads.json object */
bool threads_from_json(Thread* out, cJSON* in);

/* Free all the allocated strings by fill_thread_from_id() in a Thread array */
void threads_free(Thread* arr);

#endif /* UTIL_H_ */
