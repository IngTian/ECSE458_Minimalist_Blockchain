#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SOCKET_UTILS_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_SOCKET_UTILS_H

char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length);
int send_model_by_socket(char *server_address_str, int server_port, char *send_data, int send_size);

#endif