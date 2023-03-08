#include "sys_utils.h"

#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

/// Convert seconds to milliseconds
#define SEC_TO_MS(sec) ((sec)*1000)
/// Convert seconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec)*1000000000)

/// Convert nanoseconds to seconds
#define NS_TO_SEC(ns) ((ns) / 1000000000)
/// Convert nanoseconds to milliseconds
#define NS_TO_MS(ns) ((ns) / 1000000)
/// Convert nanoseconds to microseconds
#define NS_TO_US(ns) ((ns) / 1000)

/**
 * Get the timestamp now in ns.
 * @return Timestamp in ns.
 * @auhtor Ing Tian
 */
unsigned long get_timestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return SEC_TO_NS((unsigned long)ts.tv_sec) + ts.tv_nsec;
}

/**
 * Get the current timestamp in seconds.
 * @return The current timestamp.
 * @author Junjian Chen
 */
int get_current_unix_time() {
    time_t tick;
    tick = time(NULL);
    localtime(&tick);
    return (int)tick;
}

/**
 * Return the timestamp in a specific format.
 * @param format The format of the timestamp.
 * @param time_str_length Length of the result string.
 * @return The formatted timestamp.
 * @auhtor Ing Tian
 */
char *get_str_timestamp(char *format, unsigned int time_str_length) {
    time_t timer;
    char *buffer = (char *)malloc(time_str_length);
    struct tm *tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, time_str_length, format, tm_info);

    return buffer;
}

char* str_trim(char* str) {
    unsigned long str_len = strlen(str);
    unsigned long front_white_spaces, back_white_spaces;

    // Get number of front white spaces.
    for(front_white_spaces = 0; front_white_spaces < str_len; front_white_spaces++)
        if (!isspace(str[front_white_spaces]))
            break;

    // Get number of back white spaces.
    for(back_white_spaces = 0; str_len - 1 - back_white_spaces >= 0; back_white_spaces++)
        if (!isspace(str[str_len - 1 - back_white_spaces]))
            break;

    char *res = (char*)malloc(str_len - front_white_spaces - back_white_spaces);
    memcpy(res, &str[front_white_spaces], str_len-front_white_spaces-back_white_spaces);
    return res;
}