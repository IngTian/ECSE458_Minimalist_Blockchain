#include "sys_utils.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

/**
 * Get the timestamp now in milliseconds.
 * @return Timestamp in milliseconds.
 * @auhtor Ing Tian
 */
int get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
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
    puts(buffer);

    return buffer;
}
