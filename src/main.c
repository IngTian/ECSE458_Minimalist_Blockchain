#include <stdlib.h>

#include "model/block/block.h"
#include "utils/log_utils.h"

#define LOG_SCOPE "performance test"

int main() {
    int block_list_length = 2;
    char *file_name = "/Users/olinayu/Desktop/ko.txt";
    block *block_list[block_list_length];
    for (int i = 0; i < block_list_length; i++) block_list[i] = malloc(sizeof(block));
    generate_dot_representation(block_list, block_list_length, file_name);
    for (int i = 0; i < block_list_length; i++) free(block_list[i]);
    return 0;
}
