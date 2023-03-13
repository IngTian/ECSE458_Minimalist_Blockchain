#include "socket_util.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log_utils.h"
#include "sys_utils.h"

char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length) {
    size_t n = command_length + data_length;
    char *s = malloc(n);
    memcpy(s, command, command_length);
    memcpy(s + command_length, data, data_length);
    return s;
}

int send_model_by_socket(char *server_address_str, int server_port, char *send_data, int send_size) {
#define LOG_SCOPE "Miner"

    // create the socket
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Socket creation error. \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    // Convert IPv4 and IPv6 addresses from text to binary, set the ip address.
    if (inet_pton(AF_INET, server_address_str, &serv_addr.sin_addr) <= 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Invalid address/ Address not supported! \n");
        return -1;
    }

    // Connect the socket.
    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Connection Failed. \n");
        return -1;
    } else{
        general_log(LOG_SCOPE, LOG_INFO, "Connection to Listener.. \n");
    }

    send(sock, send_data, send_size, 0);
    general_log(LOG_SCOPE, LOG_INFO, "Client: model sent. Timestamp: %ul", get_timestamp());

    // closing the connected socket
    free(send_data);
    close(client_fd);
}