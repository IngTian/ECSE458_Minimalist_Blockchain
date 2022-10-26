#include "sys_utils.h"

#include <sys/time.h>
#include <stddef.h>

/**
 * Get the timestamp now in milliseconds.
 * @return Timestamp in milliseconds.
 */
int get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}
