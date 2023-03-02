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
#include "signal.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pthread.h"
#include "utils/log_utils.h"

#define PORT 8080
#define LOG_SCOPE "Server"

void DieWithError(char *errorMessage);
//void HandleTCPClient(int clntSocket);
void* HandleTCPClient(void* arg);

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
    int serverSock, clientSock;
    int server_fd, valread;
    struct sockaddr_in echo_client_address;
    struct sockaddr_in echo_server_address;
    int cli_addr_len;
    int opt = 1;
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

    memset(&echo_server_address, 0, sizeof(echo_server_address));
    echo_server_address.sin_family = AF_INET;
    echo_server_address.sin_addr.s_addr = INADDR_ANY;
    echo_server_address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&echo_server_address, sizeof(echo_server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //keep running for listening
    while (true){
        cli_addr_len = sizeof(echo_server_address);
        //wait for connect
        if ((clientSock = accept(server_fd, (struct sockaddr *)&echo_server_address, (socklen_t *)&cli_addr_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Handling client %s\n", inet_ntoa(echo_server_address.sin_addr));
//        HandleTCPClient(clientSock);
        pthread_t thread_id;
        int *arg = malloc(sizeof(*arg));
        *arg = clientSock;
        pthread_create(&thread_id, NULL, HandleTCPClient, arg);
    }



    //start new handler thread
//    struct sigaction handler;
//    handler.sa_handler = InterruptSignalHandler;
//    if (sigfillset(&handler.sa_mask) < 0){
//        printf("%s\n","sigfillset() failed");
//    }
//    handler.sa_flags = 0;
//    if (sigaction(SIGINT, &handler, 0) < 0)
//        printf("%s\n", "failed");
//    for(;;) pause();

    printf("Hello message sent\n");

    // closing the connected socket
    close(serverSock);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    printf("server disconnect!!! \n");
    return 0;
}

void InterruptHandler (int signalType) {
    printf("Interrupt received.Exiting program.\n");
    exit(1);
}

void DieWithError(char *errorMessage){
    printf("%s\n",errorMessage);
}


void* HandleTCPClient(void* arg){
    int clientSock = ((int *)arg)[0];
    printf("%d\n", clientSock);
    char echoBuffer[8092];

    recv(clientSock, echoBuffer, sizeof(echoBuffer), 0);
    char *command = echoBuffer;
    char *data = command + 32;
    printf("%s\n", command);


    socket_block *recv_socket_blk = (socket_block *)malloc(sizeof(socket_block )+((socket_block *)data)->txns_size);
//    socket_transaction *recv_socket_tx = (socket_transaction *)malloc();
    memcpy(recv_socket_blk, data, sizeof(socket_block )+((socket_block *)data)->txns_size);
    block* blk= cast_to_block(recv_socket_blk);
    printf("Print Recevied Block Information: \n");
    printf("Block txns count: %d\n",blk->txn_count);
    printf("Block header version: %d\n",blk->header->version);
    printf("Block header hash: ");
    print_hex(blk->header->prev_block_header_hash,64);
    printf("Block txns[0] in[0] signature script: ");
    print_hex(blk->txns[0]->tx_ins[0].signature_script,64);

//    send(clientSock, &recvPerson, sizeof(recvPerson), 0);
    close(clientSock);
}