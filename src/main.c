#include "model/transaction/transaction.h"
#include "model/transaction/transaction_persistence.h"
#include "model/block/block.h"
#include "model/block/block_persistence.h"
#include "utils/cryptography.h"
#include "utils/mysql_util.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"
#include "utils/constants.h"

transaction *create_a_new_single_in_single_out_transaction(
    char *previous_transaction_id, char *previous_output_private_key, int previous_tx_output_idx, int previous_value, char **res_txid, char **res_private_key);

block* create_a_new_block(char* previous_block_header_hash, transaction* transaction, char** result_header_hash);

#define LOG_SCOPE "performance test"

int main() {
    initialize_mysql_system();
    destroy_transaction_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *previous_transaction = initialize_transaction_system();
    char *previous_transaction_id = get_transaction_txid(previous_transaction);
    char *previous_output_private_key = get_genesis_transaction_private_key();
    char *res_txid;
    char *res_private_key;
    transaction *transaction = create_a_new_single_in_single_out_transaction(previous_transaction_id, previous_output_private_key, 0, TOTAL_NUMBER_OF_COINS, &res_txid, &res_private_key);

    //print transaction info
    printf("%d\n", transaction->tx_out_count);
    printf("%d\n", transaction->tx_in_count);
    printf("%ld\n", transaction->lock_time);
    printf("%d\n", transaction->version);
    print_hex(transaction->tx_ins[0].signature_script, 64);

    destroy_block_system();
    block *genesis_block = initialize_block_system();
    append_transaction_into_block(genesis_block, get_genesis_transaction(), 0);
    finalize_block(genesis_block);

    char* result_block_hash;
    block* block1 = create_a_new_block(get_genesis_block_hash(), transaction, &result_block_hash);

    //    print block info
    printf("Block txns count: %d\n", block1->txn_count);
    printf("Block header version: %d\n", block1->header->version);
    printf("Block header hash: ");
    print_hex(block1->header->prev_block_header_hash, 64);
    printf("Block txns[0] in[0] signature script: ");
    print_hex(block1->txns[0]->tx_ins[0].signature_script, 64);

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