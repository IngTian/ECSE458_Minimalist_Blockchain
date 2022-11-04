#include <stdlib.h>

#include "model/transaction/transaction.h"
#include "model/block//block.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "performance test"

int main() {
    // Initialize the whole system.
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

    transaction *previous_transaction = initialize_transaction_system();
    char *previous_output_private_key = get_genesis_transaction_private_key();

    block* genesis_block=initialize_block_system();
    block* previous_block=genesis_block;


    char* genesis_block_hash = get_genesis_block_hash();

    // Start testing.

    long int verify_tx_time = 0;
    long int verify_tx_signature = 0;
    long int verify_block = 0;
    long int hash_time= 0;



    for (int i = 0; i < 50000; i++) {
        char *previous_transaction_id = get_transaction_txid(previous_transaction);
        transaction_create_shortcut_input input = {.previous_output_idx = 0,
                                                   .previous_txid = previous_transaction_id,
                                                   .private_key = previous_output_private_key};
        unsigned char *new_private_key = get_a_new_private_key();
        secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
        transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS,
                                                     .public_key = (char *)new_public_key->data};

        transaction_create_shortcut create_data = {
            .num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};


        transaction *t = (transaction *)malloc(sizeof(transaction));
//        print_all_transactions();

        if (!create_new_transaction_shortcut(&create_data, t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a transaction.");
        }
        if (!finalize_transaction(t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction.");
        }

        previous_transaction = t;
        previous_output_private_key = (char *)new_private_key;

        /** Verify transaction input/outputs **/

        long int start_time = get_timestamp();

        verify_transaction(t);

        long int end_time = get_timestamp();

        verify_tx_time+=(end_time-start_time);

        /** Verify transaction cryptography **/

        start_time = get_timestamp();

        verify_transaction_cryptography(t);

        end_time = get_timestamp();

        verify_tx_signature+=(end_time-start_time);


        block* b= create_an_empty_block(10);
        b->header->version=i+5;
        append_prev_block(previous_block,b);
        append_transaction_into_block(b,t,0);
        finalize_block(b);
        previous_block=b;

    }

    long int start_time = get_timestamp();

    verify_block_chain(previous_block);

    long int end_time = get_timestamp();

    long int verify_block_time=end_time-start_time;




    start_time = get_timestamp();

    char* msg_to_hash="hello,world";

    for(int i=0; i<10000; i++){
        msg_to_hash=hash_struct_in_hex(msg_to_hash, sizeof(msg_to_hash));
    }

    end_time = get_timestamp();

    hash_time=end_time-start_time;

//    print_block_chain(previous_block);

    // Record time consumed.
    general_log(LOG_SCOPE, LOG_INFO, "Time consumed to verify transaction= %ld ms", verify_tx_time);
    general_log(LOG_SCOPE, LOG_INFO, "Time consumed to verify transaction signature= %ld ms", verify_tx_signature);
    general_log(LOG_SCOPE, LOG_INFO, "Time consumed to verify blocks= %ld ms", verify_block_time);
    general_log(LOG_SCOPE, LOG_INFO, "Time consumed to hash= %ld ms", hash_time);
}