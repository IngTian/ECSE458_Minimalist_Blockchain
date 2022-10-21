#include "log_utils.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Convert a char array of specified length to a string
 * in hexadecimal format, for printing and logging.
 * @param ptr Pointer to the char array.
 * @param byte_length The length of the byte array.
 * @return The corresponding string in hexadecimal format.
 */
char *convert_char_hexadecimal(char *ptr, unsigned int byte_length) {
    char *ret_val = (char *)malloc(2 * byte_length);
    char *ret_val_counter = ret_val;
    for (int i = 0; i < byte_length; i++) {
        sprintf(ret_val_counter, "%02hhX", *ptr++);
        ret_val_counter += 2;
    }
    *ret_val_counter = '\0';
    return ret_val;
}
