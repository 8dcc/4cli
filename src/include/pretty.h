
#ifndef PRETTY_H_
#define PRETTY_H_ 1

#include <stdbool.h>

#include <cjson/cJSON.h>

/*
 * Print the contents of a specific thread JSON to the standard output.
 *
 * TODO: Print to specificed file.
 */
bool pretty_print_thread(cJSON* thread_json);

#endif /* PRETTY_H_ */
