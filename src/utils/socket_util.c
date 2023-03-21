#include "socket_util.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log_utils.h"
#include "sys_utils.h"
#include "constants.h"

/**
 * Initialize the client socket
 * @param server_address_str sever ip address
 * @param server_port server port
 * @param sock socket value for output
 * @param client_fd client socket id for output
 * @return status value, if fail returns -1
 */
int initialize_socket(char *server_address_str, int server_port, int* sock, int* client_fd){
#define LOG_SCOPE "Miner"
    // create the socket
    int  sock_temp= 0, client_fd_temp;
    struct sockaddr_in serv_addr;
    if ((sock_temp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Socket creation error.");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    struct hostent *server = gethostbyname(server_address_str);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Convert IPv4 and IPv6 addresses from text to binary, set the ip address.
    if (inet_pton(AF_INET, server_address_str, &serv_addr.sin_addr) <= 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Invalid address/ Address not supported!");
        return -1;
    }

    // Connect the socket.
    if ((client_fd_temp = connect(sock_temp, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Connection Failed. Error Number: %d.", errno);
        return -1;
    } else {
        general_log(LOG_SCOPE, LOG_INFO, "Connection to Listener.. ");
    }
    *sock = sock_temp;
    *client_fd = client_fd_temp;
    return 0;
}

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
 * Send socket.
 * @param sock socket number
 * @param command command to tell the sever what actions should be done
 * @param block1 the block model
 * @param transaction the transaction model
 * @return
 */
int send_socket(int sock, char* command, block* block1, transaction* transaction){
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
    send(sock, send_data, send_size, 0);
//    general_log(LOG_SCOPE, LOG_INFO, "Client: model sent. Timestamp: %lu", get_timestamp());
    free(send_data);
}