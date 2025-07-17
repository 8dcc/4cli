
#ifndef MAIN_H_
#define MAIN_H_ 1

/*
 * Compile-time configuration.
 */
#define BOARD       "g"
#define THREADS_URL "https://a.4cdn.org/" BOARD "/threads.json"

/*
 * Maximum number of threads (not posts) to parse and print.
 */
#define MAX_THREADS 255

/*
 * Entry point of the program.
 */
int main(void);

#endif /* MAIN_H_ */
