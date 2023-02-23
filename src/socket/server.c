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

#define PORT 8080

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

//void HandleTCPClient(int clientSock){
//    char echoBuffer[1024];
//    ssize_t recvMsgSize;
//    transaction recvTx;
//    struct UsrData recvUser;
//    struct Person recvPerson;
//
//    //receiver message from client
//    if ((recvMsgSize = recv(clientSock, echoBuffer, 1024, 0)) < 0)
//        DieWithError("recv() failed");
//    sleep(1);
//    memcpy(&recvPerson,echoBuffer, sizeof(recvPerson));
//    recvUser=recvPerson.usrData;
//    printf("%s\n",recvUser.usr_id);
//    printf("%s\n", recvUser.usr_nickname);
//    memcpy(recvUser.usr_nickname, "Joeeeee", sizeof("Joeeeee"));
//    send(clientSock, &recvPerson, sizeof(recvPerson), 0);
//    close(clientSock);
//}

void* HandleTCPClient(void* arg){
    int clientSock = ((int *)arg)[0];
    printf("%d\n", clientSock);
    char echoBuffer[1024];
    ssize_t recvMsgSize;
    transaction recvTx;
    struct UsrData recvUser;
    struct Person recvPerson;

    //receiver message from client
    if ((recvMsgSize = recv(clientSock, echoBuffer, 1024, 0)) < 0)
        DieWithError("recv() failed");
    printf("receive message size: %d\n", (int)recvMsgSize);
    sleep(5);
    memcpy(&recvPerson,echoBuffer, sizeof(recvPerson));
    recvUser=recvPerson.usrData;
    printf("%s\n",recvUser.usr_id);
    printf("%s\n", recvUser.usr_nickname);
    memcpy(recvUser.usr_nickname, "Joeeeee", sizeof("Joeeeee"));
    send(clientSock, &recvPerson, sizeof(recvPerson), 0);
    close(clientSock);
}
