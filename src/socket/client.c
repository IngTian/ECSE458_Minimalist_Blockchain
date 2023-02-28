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

#define LOG_SCOPE "Client"
#define PORT 8080

char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length);

int main(int argc, char const *argv[]) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

    transaction *previous_transaction = initialize_transaction_system();
    char *previous_output_private_key = get_genesis_transaction_private_key();

    block *genesis_block = initialize_block_system();
    block *block_under_test;

    char *previous_block_header_hash = get_genesis_block_hash();

    // Start testing.

    for (int i = 0; i < 5; i++) {
        char *previous_transaction_id = get_transaction_txid(previous_transaction);
        transaction_create_shortcut_input input = {
            .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = previous_output_private_key};
        unsigned char *new_private_key = get_a_new_private_key();
        secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
        transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};

        transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};

        transaction *t = (transaction *)malloc(sizeof(transaction));

        if (!create_new_transaction_shortcut(&create_data, t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a transaction.");
        }

        previous_transaction = t;
        previous_output_private_key = (char *)new_private_key;

        block_header_shortcut block_header = {
            .prev_block_header_hash = NULL, .version = i, .nonce = 0, .nBits = 0, .merkle_root_hash = NULL, .time = get_current_unix_time()};

        memcpy(block_header.prev_block_header_hash, previous_block_header_hash, 65);

        transaction **txns = malloc(sizeof(txns));
        txns[0] = t;

        transactions_shortcut txns_shortcut = {.txns = txns, .txn_count = 1};

        block_create_shortcut block_data = {.header = &block_header, .transaction_list = &txns_shortcut};

        block *block1 = (block *)malloc(sizeof(block));
        if (!create_new_block_shortcut(&block_data, block1)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a block.");
        }

        verify_block(block1);

        if (i == 3) block_under_test = block1;

        if (!finalize_transaction(t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction.");
        }

        if (!finalize_block(block1)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a block.");
        }

        previous_block_header_hash = hash_block_header(block1->header);
    }

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    char *hello = "Hello, this is message from client!!!!";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    //Print block information
    printf("Block txns count: %d\n",block_under_test->txn_count);
    printf("Block header version: %d\n",block_under_test->header->version);
    printf("Block header hash: ");
    print_hex(block_under_test->header->prev_block_header_hash,64);
    printf("Block txns[0] in[0] signature script: ");
    print_hex(block_under_test->txns[0]->tx_ins[0].signature_script,64);


    //    transaction previous
    socket_block *socket_blk = cast_to_socket_block(block_under_test);
    char command[32];
    memset(command, '\0', 32);
    memcpy(command, "create block", strlen("create block"));

    int send_size = get_socket_block_length(block_under_test) + 32;
    char *send_socket_tx = combine_data_with_command(command, 32, (const char *)socket_blk, get_socket_block_length(block_under_test));

    //    send(sock, (const char *)socket_tx, get_socket_transaction_length(socket_tx), 0);
    send(sock, send_socket_tx, send_size, 0);
    printf("---- Client: transaction sent ----\n");
    valread = read(sock, buffer, 1024);

    // closing the connected socket
    free(socket_blk);
    close(client_fd);
    return 0;
}

char *combine_data_with_command(char *command, unsigned int command_length, const char *data, unsigned int data_length) {
    size_t n = command_length + data_length;
    char *s = malloc(n);
    memcpy(s, command, command_length);
    memcpy(s + command_length, data, data_length);
    return s;
}
