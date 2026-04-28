#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "model/block/block.h"
#include "model/transaction/transaction.h"
#include "model/transaction/transaction_persistence.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"
#include "utils/socket_util.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "miner"
int main(int argc, char const *argv[]) {
    // Socket
    int socket, client_fd;

    // ----------------------------------------
    // Initialize the system.
    // ----------------------------------------
    initialize_socket("127.0.0.1", atoi(SERVER_PORT_NO), &socket, &client_fd);
    initialize_mysql_system(MYSQL_DB_MINER);
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    destroy_transaction_system(MYSQL_DB_MINER);
    destroy_block_system(MYSQL_DB_MINER);
    transaction *previous_transaction = initialize_transaction_system(false);
    block *genesis_block = initialize_block_system(false);
    append_transaction_into_block(genesis_block, get_genesis_transaction(), 0);
    finalize_block(genesis_block);

    char *send_data_arr[NUMBER_OF_TEST_MODEL + 1];
    int data_size_arr[NUMBER_OF_TEST_MODEL + 1];
    int each_model_size = 0;
    if (TEST_TRANSACTION_TYPE == 0) {
        each_model_size = sizeof(socket_transaction) + 1 * sizeof(socket_transaction_input) + 1 * sizeof(socket_transaction_output);
    } else if (TEST_TRANSACTION_TYPE == 1) {
        each_model_size =
            sizeof(socket_transaction) + NUMBER_OF_TEST_TRANSACTION_INPUT * sizeof(socket_transaction_input) + 1 * sizeof(socket_transaction_output);
    } else {
        each_model_size =
            sizeof(socket_transaction) + 1 * sizeof(socket_transaction_input) + NUMBER_OF_TEST_TRANSACTION_OUTPUT * sizeof(socket_transaction_output);
    }
    int each_data_size = 0;
    if (TEST_CREATE_BLOCK) {
        // size = number of model * (size for each block + size for each tx + command length) + size of genesis
        each_data_size = sizeof(socket_block) + each_model_size + 32;
        send_data_arr[0] = (char *)malloc(428);
        for (int i = 0; i < NUMBER_OF_TEST_MODEL; i++) {
            send_data_arr[i + 1] = (char *)malloc(each_data_size);
        }
        add_send_data("genesis block", genesis_block, NULL, &send_data_arr[0], &data_size_arr[0]);
    } else {
        // size = number of model * (size for each tx + command length) + size of genesis
        send_data_arr[0] = (char *)malloc(428);
        for (int i = 0; i < NUMBER_OF_TEST_MODEL; i++) {
            send_data_arr[i + 1] = (char *)malloc(each_data_size);
        }
        add_send_data("genesis transaction", NULL, previous_transaction, &send_data_arr[0], &data_size_arr[0]);
    }

    // Send multiple transaction/block.
    int rest_coin = TOTAL_NUMBER_OF_COINS;
    uint8_t *previous_transaction_id = get_transaction_txid(previous_transaction);
    uint8_t *previous_transaction_id_array[NUMBER_OF_TEST_TRANSACTION_INPUT];
    char *previous_output_private_key = get_genesis_transaction_private_key();
    char *previous_output_private_key_array[NUMBER_OF_TEST_TRANSACTION_OUTPUT];
    int previous_output_index_list[NUMBER_OF_TEST_TRANSACTION_INPUT];
    int previous_value[NUMBER_OF_TEST_TRANSACTION_OUTPUT];
    uint8_t *res_txid;
    char *res_private_key;
    char *res_private_key_array[NUMBER_OF_TEST_TRANSACTION_OUTPUT];
    uint8_t *previous_block_header_hash = get_genesis_block_hash();
    uint8_t *result_block_hash;

    transaction *curr_transaction;
    int current_data_index = 1;
    for (int i = 0; i < NUMBER_OF_TEST_MODEL; i++) {
        if (TEST_TRANSACTION_TYPE == 0) {
            // create the one-to-one transaction
            curr_transaction = create_a_new_single_in_single_out_transaction(
                previous_transaction_id, previous_output_private_key, 0, TOTAL_NUMBER_OF_COINS, &res_txid, &res_private_key);
            memcpy(previous_transaction_id, res_txid, 32);
            memcpy(previous_output_private_key, res_private_key, 64);
        } else if (TEST_TRANSACTION_TYPE == 1) {
            // create a one to many transaction first
            rest_coin = rest_coin - (NUMBER_OF_TEST_TRANSACTION_OUTPUT - 1);
            previous_value[0] = rest_coin;
            for (int j = 1; j < NUMBER_OF_TEST_TRANSACTION_OUTPUT; j++) previous_value[j] = 1;
            transaction *t = create_a_new_single_in_many_out_transaction(previous_transaction_id,
                                                                         previous_output_private_key,
                                                                         0,
                                                                         previous_value,
                                                                         &res_txid,
                                                                         (char ***)&res_private_key_array,
                                                                         NUMBER_OF_TEST_TRANSACTION_OUTPUT);
            memcpy(previous_output_private_key_array, res_private_key_array, NUMBER_OF_TEST_TRANSACTION_OUTPUT * sizeof(char *));
            for (int j = 0; j < NUMBER_OF_TEST_TRANSACTION_INPUT; j++) previous_transaction_id_array[j] = res_txid;

            if (TEST_CREATE_BLOCK) {
                // create the block
                block *block1 = create_a_new_block(previous_block_header_hash, t, &result_block_hash);
                add_send_data("create block", block1, NULL, &send_data_arr[i], &data_size_arr[i]);
            } else {
                add_send_data("create transaction", NULL, t, &send_data_arr[i], &data_size_arr[i]);
            }

            // create multi-to-one curr_transaction
            for (int j = 0; j < NUMBER_OF_TEST_TRANSACTION_INPUT; j++) previous_output_index_list[j] = j;
            rest_coin = rest_coin + NUMBER_OF_TEST_TRANSACTION_INPUT - 1,
            curr_transaction = create_a_new_many_in_single_out_transaction(previous_transaction_id_array,
                                                                           previous_output_private_key_array,
                                                                           previous_output_index_list,
                                                                           rest_coin,
                                                                           &res_txid,
                                                                           &res_private_key,
                                                                           NUMBER_OF_TEST_TRANSACTION_INPUT);
            memcpy(previous_transaction_id, res_txid, 32);
            memcpy(previous_output_private_key, res_private_key, 64);
        } else if (TEST_TRANSACTION_TYPE == 2) {
            // create one-to-multi curr_transaction
            rest_coin = rest_coin - (NUMBER_OF_TEST_TRANSACTION_OUTPUT - 1);
            previous_value[0] = rest_coin;
            for (int j = 1; j < NUMBER_OF_TEST_TRANSACTION_OUTPUT; j++) previous_value[j] = 1;
            curr_transaction = create_a_new_single_in_many_out_transaction(previous_transaction_id,
                                                                           previous_output_private_key,
                                                                           0,
                                                                           previous_value,
                                                                           &res_txid,
                                                                           (char ***)&res_private_key_array,
                                                                           NUMBER_OF_TEST_TRANSACTION_OUTPUT);
            memcpy(previous_transaction_id, res_txid, 32);
            previous_output_private_key = res_private_key_array[0];
        } else {
            general_log(LOG_SCOPE, LOG_ERROR, "Type of test transaction is invalid!");
        }

        if (TEST_CREATE_BLOCK) {
            // create the block
            block *block1 = create_a_new_block(previous_block_header_hash, curr_transaction, &result_block_hash);
            add_send_data("create block", block1, NULL, &send_data_arr[i], &data_size_arr[i]);
            free(block1);
        } else {
            add_send_data("create transaction", NULL, curr_transaction, &send_data_arr[i], &data_size_arr[i]);
            free(curr_transaction);
        }
    }

    for (int i = 0; i < NUMBER_OF_TEST_MODEL + 1; i++) {
        send(socket, send_data_arr[i], data_size_arr[i], 0);
    }
    free(send_data_arr);
    free(data_size_arr);
    close(client_fd);
    return 0;
}