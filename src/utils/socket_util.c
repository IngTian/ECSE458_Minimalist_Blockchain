#include <stdlib.h>
#include <string.h>
#include "socket_util.h"

char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length) {
    size_t n = command_length + data_length;
    char *s = malloc(n);
    memcpy(s, command, command_length);
    memcpy(s + command_length, data, data_length);
    return s;
}