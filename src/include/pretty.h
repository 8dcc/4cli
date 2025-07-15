
#ifndef PRETTY_H_
#define PRETTY_H_ 1

#include <stdbool.h>

#include <curl/curl.h>

#include "thread.h"

/* Print thread information */
bool print_thread_info(CURL* curl, ThreadId id);

#endif /* PRETTY_H_ */
