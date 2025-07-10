
#ifndef MAIN_H_
#define MAIN_H_ 1

#include <curl/curl.h>

#define BOARD       "g"
#define THREADS_URL "https://a.4cdn.org/" BOARD "/threads.json"

extern CURL* curl;

#endif /* MAIN_H_ */
