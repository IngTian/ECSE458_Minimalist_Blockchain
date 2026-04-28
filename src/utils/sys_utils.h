#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SYS_UTILS_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SYS_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned long get_timestamp();
int get_current_unix_time();
char *get_str_timestamp(char *, unsigned int);
char *str_trim(char *str);

#ifdef __cplusplus
}
#endif

#endif
