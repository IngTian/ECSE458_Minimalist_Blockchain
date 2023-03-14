#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include "../model/block/block.h"
#include "../model/block/block_persistence.h"
#include "../model/transaction/transaction_persistence.h"
#include "pthread.h"
#include "signal.h"
#include "utils/constants.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "Listener"

void DieWithError(char *errorMessage);
void *HandleTCPClient(void *arg);

int main(int argc, char const *argv[]) {
    int serverSock, clientSock;
    int server_fd;
    struct sockaddr_in echo_client_address;
    struct sockaddr_in echo_server_address;
    int cli_addr_len;
    int opt = 1;

    char *server_address_str = "127.0.0.1";
    int echo_server_port = 8080;
    if (argc > 1) {
        server_address_str = (char *)argv[1];
        char *p;
        errno = 0;
        long conv = strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN) {
            general_log(LOG_SCOPE, LOG_ERROR, "Input port is invalid!");
        } else {
            echo_server_port = (int)conv;
            general_log(LOG_SCOPE, LOG_INFO, "Server port: %d", echo_server_port);
        }
    }
    struct addrinfo hints, *servinfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP address

    if ((rv = getaddrinfo(NULL, "8080", &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((server_fd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(server_fd);
            perror("bind");
            continue;
        }
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        break; // if we get here, we must have connected successfully
    }

//    // Creating socket file descriptor
//    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//        perror("socket failed");
//        exit(EXIT_FAILURE);
//    }
//    // Forcefully attaching socket to the port 8080
//    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
//        perror("setsockopt");
//        exit(EXIT_FAILURE);
//    }

//    memset(&echo_server_address, 0, sizeof(echo_server_address));
//    echo_server_address.sin_family = AF_INET;
//    //    echo_server_address.sin_addr.s_addr = INADDR_ANY;
//    echo_server_address.sin_port = htons(echo_server_port);
//
//    if (inet_pton(AF_INET, server_address_str, &echo_server_address.sin_addr) <= 0) {
//        printf("\nInvalid address/ Address not supported \n");
//        general_log(LOG_SCOPE, LOG_ERROR, "Invalid address/ Address not supported \n");
//        return -1;
//    }

    // Forcefully attaching socket to the port
//    if (bind(server_fd, (struct sockaddr *)&echo_server_address, sizeof(echo_server_address)) < 0) {
//        perror("bind failed");
//        exit(EXIT_FAILURE);
//    }
//    if (listen(server_fd, 3) < 0) {
//        perror("listen");
//        exit(EXIT_FAILURE);
//    }

    // link to the database
    initialize_mysql_system(MYSQL_DB_LISTENER);
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    destroy_transaction_system();
    destroy_block_system();
    initialize_transaction_system(true);
    initialize_block_system(true);

    // keep running for listening
    while (true) {
        cli_addr_len = sizeof(echo_server_address);
        // wait for connect
        if ((clientSock = accept(server_fd, (struct sockaddr *)&echo_server_address, (socklen_t *)&cli_addr_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //        printf("Handling client %s\n", inet_ntoa(echo_server_address.sin_addr));
        general_log(LOG_SCOPE, LOG_INFO, "Handling client %s", inet_ntoa(echo_server_address.sin_addr));
        pthread_t thread_id;
        int *arg = malloc(sizeof(*arg));
        *arg = clientSock;
        pthread_create(&thread_id, NULL, HandleTCPClient, arg);
    }

    // closing the connected socket
    close(serverSock);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    general_log(LOG_SCOPE, LOG_INFO, "server disconnect!!!");
    return 0;
}

void InterruptHandler(int signalType) {
    general_log(LOG_SCOPE, LOG_ERROR, "Interrupt received.Exiting program.\n");
    exit(1);
}

void DieWithError(char *errorMessage) { general_log(LOG_SCOPE, LOG_ERROR, "%s", errorMessage); }

void *HandleTCPClient(void *arg) {
    int clientSock = ((int *)arg)[0];
    char echoBuffer[8092];

    recv(clientSock, echoBuffer, sizeof(echoBuffer), 0);
    char *receiveCommand = echoBuffer;
    char *data = receiveCommand + 32;
    general_log(LOG_SCOPE, LOG_INFO, "Server: model received, Timestamp: %ul", get_timestamp());
    general_log(LOG_SCOPE, LOG_INFO, "Receive the command: %s: ", receiveCommand);

    if (TEST_CREATE_BLOCK) {
        // receive the block
        socket_block *recv_socket_blk = (socket_block *)malloc(sizeof(socket_block) + ((socket_block *)data)->txns_size);
        memcpy(recv_socket_blk, data, sizeof(socket_block) + ((socket_block *)data)->txns_size);
        block *block1 = cast_to_block(recv_socket_blk);

        // print block info
        printf("Block txns count: %d\n", block1->txn_count);
        printf("Block header version: %d\n", block1->header->version);
        printf("Block header hash: ");
        print_hex(block1->header->prev_block_header_hash, 64);
        printf("Block txns[0] in[0] signature script: ");
        print_hex(block1->txns[0]->tx_ins[0].signature_script, 64);

        receiveCommand = str_trim(receiveCommand);
        if (strcmp(receiveCommand, "genesis block") != 0) {
            // verification
            verify_block(block1);
            general_log(LOG_SCOPE, LOG_INFO, "Block verification done. Timestamp: %ul", get_timestamp());
        }
        free(receiveCommand);

        // save to database
        save_block(block1);
    } else {
        // receive the transaction
        socket_transaction *recv_socket_tx = (socket_transaction *)malloc(get_socket_transaction_length((socket_transaction *)data));
        memcpy(recv_socket_tx, data, get_socket_transaction_length((socket_transaction *)data));
        transaction *tx = cast_to_transaction(recv_socket_tx);

        // print receive socket tx info
        printf("%d\n", tx->tx_out_count);
        printf("%d\n", tx->tx_in_count);
        printf("%u\n", tx->lock_time);
        print_hex(tx->tx_ins[0].signature_script, 64);

        receiveCommand = str_trim(receiveCommand);
        if (strcmp(receiveCommand, "genesis transaction") != 0) {
            // verification
            verify_transaction(tx);
            general_log(LOG_SCOPE, LOG_INFO, "Transaction verification done. Timestamp: %ul", get_timestamp());
        }
        free(receiveCommand);

        // save to database
        save_transaction(tx);
    }

    close(clientSock);
    pthread_exit(NULL);
}
