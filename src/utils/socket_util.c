#include "socket_util.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log_utils.h"
#include "sys_utils.h"
#include "constants.h"

/**
 * Combine the model data with the command
 * @param command indicate the type of data in the socket
 * @param command_length command length
 * @param data model data
 * @param data_length model data length
 * @return a char pointer with the combined command and data.
 * @author Shichang Zhang
 */
char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length) {
    size_t n = command_length + data_length;
    char *s = malloc(n);
    memcpy(s, command, command_length);
    memcpy(s + command_length, data, data_length);
    return s;
}

/**
 * Send the model by socket
 * @param server_address_str server address
 * @param server_port server port
 * @param send_data the data to be sent
 * @param send_size the size of the data to be sent
 * @return return 0 if send successfully.
 * @author Shichang Zhang
 */
int send_model_by_socket(char *server_address_str, int server_port, char *send_data, int send_size) {
#define LOG_SCOPE "Miner"

    // create the socket
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Socket creation error.");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    // Convert IPv4 and IPv6 addresses from text to binary, set the ip address.
    if (inet_pton(AF_INET, server_address_str, &serv_addr.sin_addr) <= 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Invalid address/ Address not supported!");
        return -1;
    }

    // Connect the socket.
    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Connection Failed.");
        general_log(LOG_SCOPE, LOG_INFO, "Client fd: %d", client_fd);

        return -1;
    } else{
        general_log(LOG_SCOPE, LOG_INFO, "Connection to Listener..");
    }

    send(sock, send_data, send_size, 0);
    general_log(LOG_SCOPE, LOG_INFO, "Client: model sent. Timestamp: %ul", get_timestamp());

    // closing the connected socket
    free(send_data);
    close(client_fd);
}

int send_socket(char* command, block* block1, transaction* transaction, char* server_address_str, int server_port){
    const char* send_model;
    int send_size;
    char sendCommand[32];  // the command to tell listener to accept a block or transaction
    memset(sendCommand, '\0', 32);
    memcpy(sendCommand, command, strlen(command));
    if (TEST_CREATE_BLOCK){
        socket_block* socket_blk = cast_to_socket_block(block1);
        send_model = (const char *)socket_blk;
        send_size = get_socket_block_length(block1) + COMMAND_LENGTH;
    }else{
        socket_transaction * socket_tx = cast_to_socket_transaction(transaction);
        send_model = (const char *)socket_tx;
        send_size = get_socket_transaction_length(socket_tx) + COMMAND_LENGTH;
    }
    char* send_data = combine_data_with_command(sendCommand, COMMAND_LENGTH, send_model, send_size);
    send_model_by_socket(server_address_str, server_port, send_data, send_size);
    usleep(1000);  // sleep for 1ms
}