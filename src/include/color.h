#ifndef COLOR_H_
#define COLOR_H_ 1

/* Bold colors */
#define TC_B_NRM "\x1B[1m"
#define TC_B_GRY "\x1B[1;30m"
#define TC_B_RED "\x1B[1;31m"
#define TC_B_GRN "\x1B[1;32m"
#define TC_B_YEL "\x1B[1;33m"
#define TC_B_BLU "\x1B[1;34m"
#define TC_B_MAG "\x1B[1;35m"
#define TC_B_CYN "\x1B[1;36m"
#define TC_B_WHT "\x1B[1;37m"

/* Normal colors */
#define TC_NRM "\x1B[0m"
#define TC_BLK "\x1B[0;30m"
#define TC_RED "\x1B[0;31m"
#define TC_GRN "\x1B[0;32m"
#define TC_YEL "\x1B[0;33m"
#define TC_BLU "\x1B[0;34m"
#define TC_MAG "\x1B[0;35m"
#define TC_CYN "\x1B[0;36m"
#define TC_WHT "\x1B[0;37m"

/* App colors, if enabled */
#ifdef USE_COLOR
#define COL_NORM     TC_NRM
#define COL_BOLD     TC_B_NRM
#define COL_ERROR    TC_RED
#define COL_WARN     TC_YEL
#define COL_INFO     TC_B_BLU
#define COL_TITLE    TC_B_MAG
#define COL_URL      TC_WHT
#define COL_FILENAME TC_B_CYN
#define COL_REPLIES  TC_B_WHT
#else
#define COL_NORM     ""
#define COL_BOLD     ""
#define COL_ERROR    ""
#define COL_WARN     ""
#define COL_INFO     ""
#define COL_TITLE    ""
#define COL_URL      ""
#define COL_FILENAME ""
#define COL_REPLIES  ""
#endif

#endif /* COLOR_H_ */
