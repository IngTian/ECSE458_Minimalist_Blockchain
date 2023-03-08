// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../model/block/block.h"
#include "../model/transaction/transaction.h"
#include "model/block//block.h"
#include "model/transaction/transaction.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"
#include "utils/sys_utils.h"
#include "utils/socket_util.h"

#define LOG_SCOPE "Client"

transaction *create_a_new_single_in_single_out_transaction(
    char *previous_transaction_id, char *previous_output_private_key, int previous_tx_output_idx, int previous_value, char **res_txid, char **res_private_key);

int main(int argc, char const *argv[]) {
    char *server_address_str = "127.0.0.1";
    int server_port = 8080;
    if (argc > 1) {
        server_address_str = (char *)argv[1];
        char *p;
        errno = 0;
        long conv = strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN) {
            general_log(LOG_SCOPE, LOG_ERROR, "Input port is invalid!");
        } else {
            server_port = (int)conv;
            printf("%d\n", server_port);
        }
    }

    //create the transaction
    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    destroy_transaction_system();
    transaction *previous_transaction = initialize_transaction_system();
    char *previous_transaction_id = get_transaction_txid(previous_transaction);
    char *previous_output_private_key = get_genesis_transaction_private_key();
    char *res_txid;
    char *res_private_key;
    transaction *transaction = create_a_new_single_in_single_out_transaction(previous_transaction_id, previous_output_private_key, 0, TOTAL_NUMBER_OF_COINS, &res_txid, &res_private_key);

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    // Convert IPv4 and IPv6 addresses from text to binary, set the ip address
    if (inet_pton(AF_INET, server_address_str, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    //connect the socket
    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    //print transaction info
    printf("%d\n", transaction->tx_out_count);
    printf("%d\n", transaction->tx_in_count);
    printf("%u\n", transaction->lock_time);
    printf("%d\n", transaction->version);
    print_hex(transaction->tx_ins[0].signature_script, 64);

    //send transaction
    socket_transaction *socket_tx = cast_to_socket_transaction(transaction);
    char sendCommand[32];
    memset(sendCommand, '\0', 32);
    memcpy(sendCommand, "create transaction", strlen("create transaction"));
    int send_size = get_socket_transaction_length(socket_tx)+ 32;
    char* send_socket_tx = combine_data_with_command(sendCommand,
                                                     32,
                                                     (const char*)socket_tx,
                                                     get_socket_transaction_length(socket_tx));
    send(sock, send_socket_tx, send_size, 0);
    printf("---- Client: transaction sent ----\n");

    //receive the block back
    char receiveBuffer[8092];
    recv(sock, receiveBuffer, sizeof(receiveBuffer), 0);
    char *receiveCommand = receiveBuffer;
    char *data = receiveCommand + 32;
    general_log(LOG_SCOPE, LOG_INFO, receiveCommand);
    socket_block *recv_socket_blk = (socket_block *)malloc(sizeof(socket_block )+((socket_block *)data)->txns_size);
    memcpy(recv_socket_blk, data, sizeof(socket_block )+((socket_block *)data)->txns_size);
    block* blk= cast_to_block(recv_socket_blk);
    printf("Print Recevied Block Information: \n");
    printf("Block txns count: %d\n",blk->txn_count);
    printf("Block header version: %d\n",blk->header->version);
    printf("Block header hash: ");
    print_hex(blk->header->prev_block_header_hash,64);
    printf("Block txns[0] in[0] signature script: ");
    print_hex(blk->txns[0]->tx_ins[0].signature_script,64);

    //save to database


    // closing the connected socket
    free(socket_tx);
    close(client_fd);
    return 0;
}

transaction *create_a_new_single_in_single_out_transaction(
    char *previous_transaction_id, char *previous_output_private_key, int previous_tx_output_idx, int previous_value, char **res_txid, char **res_private_key) {
    transaction_create_shortcut_input input = {
        .previous_output_idx = previous_tx_output_idx, .previous_txid = previous_transaction_id, .private_key = previous_output_private_key};

    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = previous_value, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *t = (transaction*)malloc(sizeof(transaction));

    if (!create_new_transaction_shortcut(&create_data, t)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a transaction.");
    }

    printf("Clock Time After Creation: %ld\n", t->lock_time);

    if (!finalize_transaction(t)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction.");
    }

    printf("Clock Time After Finalize: %ld\n", t->lock_time);

    *res_txid = get_transaction_txid(t);
    *res_private_key = new_private_key;

    return t;
}