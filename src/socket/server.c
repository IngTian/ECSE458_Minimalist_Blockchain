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
#include "../model/transaction/transaction_persistence.h"
#include "../model/block/block_persistence.h"
#include "signal.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pthread.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"
#include "utils/socket_util.h"

#define LOG_SCOPE "Server"

void DieWithError(char *errorMessage);
void* HandleTCPClient(void* arg);
block* create_a_new_block(char* previous_block_header_hash, transaction* transaction, char** result_header_hash);

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
            printf("%d\n", echo_server_port);
        }
    }

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
//    echo_server_address.sin_addr.s_addr = INADDR_ANY;
    echo_server_address.sin_port = htons(echo_server_port);

    if (inet_pton(AF_INET, server_address_str, &echo_server_address.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Forcefully attaching socket to the port
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
        pthread_t thread_id;
        int *arg = malloc(sizeof(*arg));
        *arg = clientSock;
        pthread_create(&thread_id, NULL, HandleTCPClient, arg);
    }

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
    char *receiveCommand = echoBuffer;
    char *data = receiveCommand + 32;
    general_log(LOG_SCOPE, LOG_INFO, "Receive the command: %s: ", receiveCommand);

    //receive the transaction
    socket_transaction *recv_socket_tx = (socket_transaction *)malloc(get_socket_transaction_length((socket_transaction *)data));
    memcpy(recv_socket_tx, data, get_socket_transaction_length((socket_transaction *)data));
    transaction *tx = cast_to_transaction(recv_socket_tx);
    printf("%d\n",tx->tx_out_count);
    printf("%d\n",tx->tx_in_count);
    printf("%u\n",tx->lock_time);
    printf("%d\n",tx->version);
    print_hex(tx->tx_ins[0].signature_script,64);

    //create the block and append transaction into it
    destroy_block_system();
    block *genesis_block = initialize_block_system();
    append_transaction_into_block(genesis_block, get_genesis_transaction(), 0);
    finalize_block(genesis_block);

    char* result_block_hash;
    block* block1 = create_a_new_block(get_genesis_block_hash(), tx, &result_block_hash);
    
//    print block info
    printf("Block txns count: %d\n", block1->txn_count);
    printf("Block header version: %d\n", block1->header->version);
    printf("Block header hash: ");
    print_hex(block1->header->prev_block_header_hash, 64);
    printf("Block txns[0] in[0] signature script: ");
    print_hex(block1->txns[0]->tx_ins[0].signature_script, 64);

    //send the block to client
    socket_block *socket_blk = cast_to_socket_block(block1);
    char sendCommand[32];
    memset(sendCommand, '\0', 32);
    memcpy(sendCommand, "create block", strlen("create block"));

    int send_size = get_socket_block_length(block1) + 32;
    char *send_socket_tx = combine_data_with_command(sendCommand, 32, (const char *)socket_blk, get_socket_block_length(block1));
    send(clientSock, send_socket_tx, send_size, 0);
    printf("---- Server: blcok sent ----\n");

    close(clientSock);
}

block* create_a_new_block(char* previous_block_header_hash, transaction* transaction, char** result_header_hash){
    block_header_shortcut block_header = {
        .prev_block_header_hash = "", .version = 0, .nonce = 0, .nBits = 0, .merkle_root_hash = "", .time = get_current_unix_time()};
    memcpy(block_header.prev_block_header_hash, previous_block_header_hash, 65);
    struct transaction** txns = malloc(sizeof(txns));
    txns[0] = transaction;
    transactions_shortcut txns_shortcut = {.txns = txns, .txn_count = 1};
    block_create_shortcut block_data = {.header = &block_header, .transaction_list = &txns_shortcut};

    block *block1 = (block *)malloc(sizeof(block));
    if (!create_new_block_shortcut(&block_data, block1)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a block.");
    }
    verify_block(block1);
    if (!finalize_block(block1)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a block.");
    }

    *result_header_hash = hash_block_header(block1->header);
    return block1;
}