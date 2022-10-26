#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_LOG_UTILS_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_LOG_UTILS_H

#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_ERROR 2

char *convert_char_hexadecimal(char *, unsigned int);
void general_log(char *, int, char *, ...);
void print_hex(unsigned char*, int size);

#endif
