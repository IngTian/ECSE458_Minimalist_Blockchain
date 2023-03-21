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
    int server_port = atoi(SERVER_PORT_NO);

    //socket
    int socket;
    int client_fd;

    // initialize system
    initialize_socket(server_address_str, server_port, &socket, &client_fd);
    initialize_mysql_system(MYSQL_DB_MINER);
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    destroy_transaction_system("miner");
    destroy_block_system("miner");
    transaction *previous_transaction = initialize_transaction_system(false);
    block *genesis_block = initialize_block_system(false);
    append_transaction_into_block(genesis_block, get_genesis_transaction(), 0);
    finalize_block(genesis_block);

    if (TEST_CREATE_BLOCK){
        // create the block
        send_socket(socket, "genesis block",genesis_block,NULL);
    }else{
        send_socket(socket, "genesis transaction",NULL,previous_transaction);
    }

    // send multiple transaction/block
    int rest_coin = TOTAL_NUMBER_OF_COINS;
    char *previous_transaction_id = get_transaction_txid(previous_transaction);
    char *previous_transaction_id_array[2];
    char *previous_output_private_key = get_genesis_transaction_private_key();
    char *previous_output_private_key_array[2];
    int previous_output_index_list[2];
    int previous_value[2];
    char *res_txid;
    char *res_private_key;
    char *res_private_key_array[2];
    char *previous_block_header_hash = get_genesis_block_hash();
    char *result_block_hash;

    transaction *transaction;
    for (int i = 0; i < NUMBER_OF_TEST_MODEL; i++) {
        if (TEST_TRANSACTION_TYPE == 0) {
            // create the one-to-one transaction
            transaction = create_a_new_single_in_single_out_transaction(
                previous_transaction_id, previous_output_private_key, 0, TOTAL_NUMBER_OF_COINS, &res_txid, &res_private_key);
            memcpy(previous_transaction_id, res_txid, 64);
            memcpy(previous_output_private_key, res_private_key, 64);
        } else if (TEST_TRANSACTION_TYPE == 1) {
            // create a one to many transaction first
            rest_coin = rest_coin - (NUMBER_OF_TEST_TRANSACTION_OUTPUT - 1);
            previous_value[0] = rest_coin;
            for (int i = 1; i < NUMBER_OF_TEST_TRANSACTION_OUTPUT; i++) previous_value[i] = 1;
            struct transaction *t = create_a_new_single_in_many_out_transaction(previous_transaction_id,
                                                                                previous_output_private_key,
                                                                                0,
                                                                                previous_value,
                                                                                &res_txid,
                                                                                &res_private_key_array,
                                                                                NUMBER_OF_TEST_TRANSACTION_OUTPUT);
            memcpy(previous_output_private_key_array, res_private_key_array, NUMBER_OF_TEST_TRANSACTION_OUTPUT * sizeof(char *));
            for (int i = 0; i < NUMBER_OF_TEST_TRANSACTION_INPUT; i++) previous_transaction_id_array[i] = res_txid;

            if (TEST_CREATE_BLOCK){
                // create the block
                block* block1 = create_a_new_block(previous_block_header_hash, t, &result_block_hash);
                send_socket(socket, "create block",block1,NULL);
            }else{
                send_socket(socket, "create transaction",NULL,t);
            }

            // create multi-to-one transaction
            for (int i = 0; i < NUMBER_OF_TEST_TRANSACTION_INPUT; i++) previous_output_index_list[i] = i;
            rest_coin = rest_coin + NUMBER_OF_TEST_TRANSACTION_INPUT - 1,
            transaction = create_a_new_many_in_single_out_transaction(previous_transaction_id_array,
                                                                      previous_output_private_key_array,
                                                                      previous_output_index_list,
                                                                      rest_coin,
                                                                      &res_txid,
                                                                      &res_private_key,
                                                                      NUMBER_OF_TEST_TRANSACTION_INPUT);
            memcpy(previous_transaction_id, res_txid, 64);
            memcpy(previous_output_private_key, res_private_key, 64);
        } else if (TEST_TRANSACTION_TYPE == 2) {
            // create one-to-multi transaction
            rest_coin = rest_coin - (NUMBER_OF_TEST_TRANSACTION_OUTPUT - 1);
            previous_value[0] = rest_coin;
            for (int i = 1; i < NUMBER_OF_TEST_TRANSACTION_OUTPUT; i++) previous_value[i] = 1;
            transaction = create_a_new_single_in_many_out_transaction(previous_transaction_id,
                                                                      previous_output_private_key,
                                                                      0,
                                                                      previous_value,
                                                                      &res_txid,
                                                                      &res_private_key_array,
                                                                      NUMBER_OF_TEST_TRANSACTION_OUTPUT);
            memcpy(previous_transaction_id, res_txid, 64);
            previous_output_private_key = res_private_key_array[0];
        } else {
            general_log(LOG_SCOPE, LOG_ERROR, "Type of test transaction is invalid!");
        }

        if (TEST_CREATE_BLOCK){
            // create the block
            block* block1 = create_a_new_block(previous_block_header_hash, transaction, &result_block_hash);
            send_socket(socket, "create block",block1,NULL);
            free(block1);
        }else{
            send_socket(socket, "create transaction",NULL,transaction);
            free(transaction);
        }
    }

    close(client_fd);
    return 0;
}
