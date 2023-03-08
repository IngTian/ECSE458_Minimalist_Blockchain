#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SYS_UTILS_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SYS_UTILS_H

unsigned long get_timestamp();
int get_current_unix_time();
char *get_str_timestamp(char *, unsigned int);
char* str_trim(char* str);

#endif
