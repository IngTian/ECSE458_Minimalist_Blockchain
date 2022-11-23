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

    char** private_key_array = malloc(40000*65);
    secp256k1_pubkey** public_key_array = malloc(40000*sizeof(secp256k1_pubkey));
    for (int i = 0; i < 40000; i++) {
        char *new_private_key = (char*) get_a_new_private_key();
        secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
        private_key_array[i] = new_private_key;
        public_key_array[i] = new_public_key;
    }
    int outputCounter = 0;
    int money_left = TOTAL_NUMBER_OF_COINS;

    // Start testing.

    for (int i = 0; i < 10000; i++) {
        char *previous_transaction_id = get_transaction_txid(previous_transaction);
        transaction_create_shortcut_input input = {.previous_output_idx = 0,
                                                   .previous_txid = previous_transaction_id,
                                                   .private_key = previous_output_private_key};

        int num_outputs = rand()%4+1;
        money_left = money_left-num_outputs+1;
        transaction_create_shortcut_output* outputs=malloc(num_outputs*sizeof(transaction_create_shortcut_output));
        transaction_create_shortcut_output output = {.value = money_left,
                                                     .public_key = (char *)(public_key_array[outputCounter]->data)};
        previous_output_private_key = private_key_array[outputCounter];
        outputCounter++;
        outputs[0] = output;

        for (int j = 1; j < num_outputs; j++) {
            transaction_create_shortcut_output output1 = {.value = 1,
                                                         .public_key = (char *)(public_key_array[outputCounter]->data)};
            outputCounter++;
            outputs[j] = output1;
        }

//        unsigned char *new_private_key = get_a_new_private_key();
//        secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
//
//        unsigned char *new_private_key1 = get_a_new_private_key();
//        secp256k1_pubkey *new_public_key1 = get_a_new_public_key((char *)new_private_key1);
//
//        transaction_create_shortcut_output output1 = {.value = TOTAL_NUMBER_OF_COINS-i-1,
//                                                     .public_key = (char *)(new_public_key->data)};
//
//        transaction_create_shortcut_output output2 = {.value = 1,
//                                                      .public_key = (char *)new_public_key1->data};
//
//        transaction_create_shortcut_output* outputs=malloc(2*sizeof(transaction_create_shortcut_output));
//
//        outputs[0]=output1;
//        outputs[1]=output2;

        transaction_create_shortcut create_data = {
            .num_of_inputs = 1, .num_of_outputs = num_outputs, .outputs = outputs, .inputs = &input};


        transaction *t = (transaction *)malloc(sizeof(transaction));



        if (!create_new_transaction_shortcut(&create_data, t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to create a transaction.");
        }

        verify_transaction(t);

        if (!finalize_transaction(t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction.");
        }

        previous_transaction = t;
//        previous_output_private_key = new_private_key;
    }
}
