#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "model/block//block.h"
#include "model/transaction/transaction.h"
#include "model/transaction/transaction_persistence.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"
#include "utils/socket_util.h"

#define LOG_SCOPE "miner"
int main(int argc, char const *argv[]) {
    // listener's address and port configuration
    char *server_address_str = "127.0.0.1";
    int server_port = SERVER_POST;
    if (argc > 1) {
        server_address_str = (char *)argv[1];
        char *p;
        errno = 0;
        long conv = strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN) {
            general_log(LOG_SCOPE, LOG_ERROR, "Input port is invalid!");
        } else {
            server_port = (int)conv;
            general_log(LOG_SCOPE, LOG_INFO, "Server port: %d", server_port);
        }
    }

    // send socket data configuration
    char sendCommand[32];  // the command to tell listener to accept a block or transaction
    memset(sendCommand, '\0', 32);
    const char *send_model;
    int send_size;
    char *send_data;
    socket_block *socket_blk = NULL;
    socket_transaction *socket_tx = NULL;

    // initialize system
    initialize_mysql_system(MYSQL_DB_MINER);
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    destroy_transaction_system();
    destroy_block_system();
    transaction *previous_transaction = initialize_transaction_system(false);
    block *genesis_block = initialize_block_system(false);
    append_transaction_into_block(genesis_block, get_genesis_transaction(), 0);
    finalize_block(genesis_block);

    if (TEST_CREATE_BLOCK) {
        // send genesis block to listener
        memcpy(sendCommand, "genesis block", strlen("genesis block"));
        socket_blk = cast_to_socket_block(genesis_block);
        send_model = (const char *)socket_blk;
        send_size = get_socket_block_length(genesis_block) + COMMAND_LENGTH;
    } else {
        // send genesis transaction to listener
        memcpy(sendCommand, "genesis transaction", strlen("genesis transaction"));
        socket_tx = cast_to_socket_transaction(previous_transaction);
        send_model = (const char *)socket_tx;
        send_size = get_socket_transaction_length(socket_tx) + COMMAND_LENGTH;
    }
    send_data = combine_data_with_command(sendCommand, COMMAND_LENGTH, send_model, send_size);
    send_model_by_socket(server_address_str, server_port, send_data, send_size);
    usleep(1000);  // sleep for 1ms

    // send multiple transaction/block
    int n = 3;
    int number_of_input = 2;
    int number_of_output = 2;
    int transaction_type = 2;
    int rest_coin = TOTAL_NUMBER_OF_COINS;
    char *previous_transaction_id = get_transaction_txid(previous_transaction);
    char *previous_output_private_key = get_genesis_transaction_private_key();
    char *previous_output_private_key_array[2];
    int previous_value[2];
    char *res_txid;
    char *res_private_key;
    char *res_private_key_array[2];
    char *previous_block_header_hash = get_genesis_block_hash();
    char *result_block_hash;

    transaction *transaction;
    for (int i = 0; i < n; i++) {
        if (transaction_type == 0) {
            // create the one-to-one transaction
            transaction = create_a_new_single_in_single_out_transaction(
                previous_transaction_id, previous_output_private_key, 0, TOTAL_NUMBER_OF_COINS, &res_txid, &res_private_key);
            memcpy(previous_transaction_id, res_txid, 64);
            memcpy(previous_output_private_key, res_private_key, 64);
        } else if (transaction_type == 1) {
            // create a one to many transaction first
            for (int i = 0; i < number_of_output - 1; i++) previous_value[i] = 1;
            previous_value[number_of_output - 1] = TOTAL_NUMBER_OF_COINS - number_of_output + 1;
            struct transaction *t = create_a_new_single_in_many_out_transaction(
                previous_transaction_id, previous_output_private_key, 0, previous_value, &res_txid, &res_private_key_array, 2);
            previous_transaction = t;
            memcpy(previous_output_private_key_array, res_private_key_array, number_of_output * sizeof(char *));
            memcpy(previous_transaction_id, res_txid, 64);
            if (TEST_CREATE_BLOCK) {
                block *block1 = create_a_new_block(previous_block_header_hash, t, &result_block_hash);
                memcpy(previous_block_header_hash, result_block_hash, 64);
                memcpy(sendCommand, "create block", strlen("create block"));
                socket_blk = cast_to_socket_block(block1);
                send_model = (const char *)socket_blk;
                send_size = get_socket_block_length(block1) + COMMAND_LENGTH;
            } else {
                memcpy(sendCommand, "create transaction", strlen("create transaction"));
                socket_tx = cast_to_socket_transaction(t);
                send_model = (const char *)socket_tx;
                send_size = get_socket_transaction_length(socket_tx) + COMMAND_LENGTH;
            }
            send_data = combine_data_with_command(sendCommand, COMMAND_LENGTH, send_model, send_size);
            send_model_by_socket(server_address_str, server_port, send_data, send_size);
            usleep(1000);  // sleep for 1ms

            // create multi-to-one transaction
            char *previous_transaction_id_list[2];
            previous_transaction_id_list[0] = get_transaction_txid(previous_transaction);
            previous_transaction_id_list[1] = get_transaction_txid(previous_transaction);
            int previous_output_index_list[2];
            previous_output_index_list[0] = 0;
            previous_output_index_list[1] = 1;
            transaction = create_a_new_many_in_single_out_transaction(previous_transaction_id_list,
                                                                      previous_output_private_key_array,
                                                                      previous_output_index_list,
                                                                      TOTAL_NUMBER_OF_COINS,
                                                                      &res_txid,
                                                                      &res_private_key,
                                                                      2);
        } else if (transaction_type == 2) {
            // create one-to-multi transaction
            rest_coin = rest_coin - (number_of_output - 1);
            previous_value[0] = rest_coin;
            for (int i = 1; i < number_of_output; i++) previous_value[i] = 1;
            transaction = create_a_new_single_in_many_out_transaction(
                previous_transaction_id, previous_output_private_key, 0, previous_value, &res_txid, &res_private_key_array, number_of_output);
            memcpy(previous_transaction_id, res_txid, 64);
            previous_output_private_key = res_private_key_array[0];
        } else {
            general_log(LOG_SCOPE, LOG_ERROR, "Type of test transaction is invalid!");
        }

        if (TEST_CREATE_BLOCK) {
            // create the block
            block *block1 = create_a_new_block(previous_block_header_hash, transaction, &result_block_hash);
            memcpy(previous_block_header_hash, result_block_hash, 64);
            memcpy(sendCommand, "create block", strlen("create block"));
            socket_blk = cast_to_socket_block(block1);
            send_model = (const char *)socket_blk;
            send_size = get_socket_block_length(block1) + COMMAND_LENGTH;
        } else {
            memcpy(sendCommand, "create transaction", strlen("create transaction"));
            socket_tx = cast_to_socket_transaction(transaction);
            send_model = (const char *)socket_tx;
            send_size = get_socket_transaction_length(socket_tx) + COMMAND_LENGTH;
        }

        // combine with command
        send_data = combine_data_with_command(sendCommand, COMMAND_LENGTH, send_model, send_size);
        // create and send the socket
        send_model_by_socket(server_address_str, server_port, send_data, send_size);
        usleep(1000);
    }

    return 0;
}
