#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SOCKET_UTILS_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SOCKET_UTILS_H

#include "model/block/block.h"
#include "model/transaction/transaction.h"

char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length);
int initialize_socket(char *server_address_str, int server_port, int* socket, int* client_fd);
int add_send_data(char* command, block* block1, transaction* transaction, char** send_data_arr, int* data_size_arr);
#endif