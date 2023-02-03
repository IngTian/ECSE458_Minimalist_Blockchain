#include <stdlib.h>
#include <strings.h>

#include "model/transaction/transaction.h"
#include "model/block//block.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "performance test"

int main() {
    // Initialize the whole system.
    mysql_config config = {.host_addr = "localhost",
                           .username = "root",
                           .password = "Admin123/",
                           .db = "test",
                           .port_number = 3306,
                           .client_flag = CLIENT_MULTI_STATEMENTS};
    initialize_mysql_system(config);
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *previous_transaction = initialize_transaction_system();
    char *previous_output_private_key = get_genesis_transaction_private_key();
    /** Create a sub-genesis transaction with 1 input from genesis transaction and 10001 outputs: 1*30960+10000*1 **/
    char *previous_transaction_id = get_transaction_txid(previous_transaction);

    int size = 5;

    // Input from genesis tx
    transaction_create_shortcut_input input_genesis = {.previous_output_idx = 0,
                                                       .previous_txid = previous_transaction_id,
                                                       .private_key = previous_output_private_key};
    //Create an output array of length 10001
    transaction_create_shortcut_output *outputs_genesis = malloc((size+1) * sizeof(transaction_create_shortcut_output));
    unsigned char* outputs_genesis_private_keys[size+1];

    //Create 10000 outputs, each one has 1 value
    for(int i=0;i<size;i++){
        unsigned char *new_private_key_output_genesis = get_a_new_private_key();
        secp256k1_pubkey *new_public_key_output_genesis = get_a_new_public_key((char *) new_private_key_output_genesis);

        transaction_create_shortcut_output output_genesis = {.value = 1,
                                                             .public_key = ((char *) (new_public_key_output_genesis->data))};

        outputs_genesis[i]=output_genesis;
        outputs_genesis_private_keys[i]=new_private_key_output_genesis;

    }

    //Create 1 output, it has 30960 value
    unsigned char *new_private_key_output_genesis_large = get_a_new_private_key();
    secp256k1_pubkey *new_public_key_output_genesis_large = get_a_new_public_key((char *) new_private_key_output_genesis_large);

    transaction_create_shortcut_output output_genesis_large = {.value = TOTAL_NUMBER_OF_COINS-size,
                                                               .public_key = ((char *) (new_public_key_output_genesis_large->data))};

    outputs_genesis[size]=output_genesis_large;
    outputs_genesis_private_keys[size]=new_private_key_output_genesis_large;

    transaction_create_shortcut create_data_genesis = {
        .num_of_inputs = 1, .num_of_outputs = size+1, .outputs = outputs_genesis, .inputs = &input_genesis};
    transaction *t_sub_genesis = (transaction *) malloc(sizeof(transaction));

    if (!create_new_transaction_shortcut(&create_data_genesis, t_sub_genesis)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a transaction.");
    } else{
        general_log(LOG_SCOPE, LOG_INFO, "create a genesis sub transaction.");
    }

    verify_transaction(t_sub_genesis);

    if (!finalize_transaction(t_sub_genesis)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction.");
    }

    previous_transaction = t_sub_genesis;

    /** Create 10000*1 one to one transactions **/

    transaction *small_txs= malloc(size* sizeof(transaction));

    char *sub_genesis_transaction_id = get_transaction_txid(previous_transaction);
    general_log(LOG_SCOPE, LOG_INFO, "Sub genesis hash: %s, its length: %d", sub_genesis_transaction_id, strlen(sub_genesis_transaction_id));
    previous_transaction_id = sub_genesis_transaction_id;
    unsigned char* previous_private_key;

    for(int i=0;i<size;i++){

        unsigned char *new_private_key_output_small = get_a_new_private_key();
        secp256k1_pubkey *new_public_key_output_small = get_a_new_public_key((char *) new_private_key_output_small);

        transaction_create_shortcut_input input_small = {.previous_output_idx = i,
                                                         .previous_txid = sub_genesis_transaction_id,
                                                         .private_key = (char*)outputs_genesis_private_keys[i]};

        transaction_create_shortcut_input input_large = {.previous_output_idx = i == 0 ? size : 0,
                                                         .previous_txid = previous_transaction_id,
                                                         .private_key = i == 0 ? (char*)outputs_genesis_private_keys[size] : (char *)previous_private_key};

        transaction_create_shortcut_output output_small = {.value = TOTAL_NUMBER_OF_COINS-size+1+i,
                                                           .public_key = ((char *)(new_public_key_output_small->data))};

        transaction_create_shortcut_input* inputs=malloc(2*sizeof(transaction_create_shortcut_input));
        inputs[0]=input_small;
        inputs[1]=input_large;

        transaction_create_shortcut create_data_small = {
            .num_of_inputs = 2, .num_of_outputs = 1, .outputs = &output_small, .inputs = inputs};

        transaction *t_small = (transaction *)malloc(sizeof(transaction));

        if (!create_new_transaction_shortcut(&create_data_small, t_small)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a transaction. + %d", i);
        }

        verify_transaction(t_small);

        if (!finalize_transaction(t_small)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction %d.",i);
        }else{
            general_log(LOG_SCOPE, LOG_INFO, "the %d round success", i);
        }

        previous_transaction_id = get_transaction_txid(t_small);
        previous_private_key = new_private_key_output_small;
    }

}
