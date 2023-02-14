// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../model/transaction/transaction.h"
#include "../model/block/block.h"

#define PORT 8080

struct UsrData {
    char usr_id[16];
    char usr_pwd[16];
    char usr_nickname[16];
};

struct Person {
    int pid;
    struct UsrData usrData;
};

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello, this is message from server!!!!";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    transaction recvTx;
    struct UsrData recvUser;
    struct Person recvPerson;

    recv(new_socket,buffer, sizeof(buffer),0);
    memcpy(&recvPerson,buffer, sizeof(recvPerson));
    recvUser=recvPerson.usrData;
//    printf("Tx version: %d\n", recvTx.version);
//    printf("Tx lock time: %d\n", recvTx.lock_time);
//    printf("Tx out count: %d\n", recvTx.tx_out_count);
//    printf("Tx in count: %d\n", recvTx.tx_in_count);
//    printf("Tx out: %d\n", recvTx.tx_outs->value);
    printf("Person pid: %d\n", recvPerson.pid);
    printf("Person user data id: %s\n", recvUser.usr_id);






    send(new_socket, buffer, strlen(hello), 0);
    printf("Hello message sent\n");

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
