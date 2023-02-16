#include <stdlib.h>
#include <strings.h>

#include "model/block//block.h"
#include "model/transaction/transaction.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "performance test"

int main() {
    // Initialize the whole system.
    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    // Initialize the whole system.e

    transaction *previous_transaction = initialize_transaction_system();
    char *previous_output_private_key = get_genesis_transaction_private_key();

    block *genesis_block = initialize_block_system();
    append_transaction_into_block(genesis_block, previous_transaction, 0);
    finalize_block(genesis_block);

    char *previous_block_header_hash = get_genesis_block_hash();

    // Start testing.

    for (int i = 0; i < 100; i++) {
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

        if (!finalize_transaction(t)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a transaction.");
        }

        if (!finalize_block(block1)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to finalize a block.");
        }

        previous_block_header_hash = hash_block_header(block1->header);
    }

    //    GList* block_list=get_all_blocks();
    //    GHashTable* utxo= generate_utxo(block_list);
    //    printf("%d\n", g_hash_table_size(utxo));
    //    print_target_utxo(utxo);
    //
    //    g_hash_table_destroy(utxo);
}
