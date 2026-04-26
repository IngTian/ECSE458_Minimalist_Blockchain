#include "../src/model/block/block.h"

#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../src/utils/cryptography.h"
#include "../src/utils/mysql_util.h"
#include "utils/constants.h"
#include "utils/sys_utils.h"

START_TEST(test_block_system_init_and_destroy) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_transaction = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_transaction, 0);
    finalize_block(genesis_b);

    ck_assert_ptr_nonnull(genesis_b);
    ck_assert_ptr_nonnull(genesis_b->header);
    ck_assert_int_eq(genesis_b->header->nBits, 0);
    ck_assert_int_eq(genesis_b->header->nonce, 0);
    ck_assert_ptr_nonnull(genesis_b->txns);
    ck_assert_int_eq(genesis_b->txn_count, 1);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_create_empty_block) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_transaction = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_transaction, 0);
    finalize_block(genesis_b);

    block *new_block = create_an_empty_block(10);
    ck_assert_ptr_nonnull(new_block);
    ck_assert_int_eq(new_block->header->version, 0);
    ck_assert_int_eq(new_block->header->nonce, 0);
    ck_assert_int_eq(new_block->txn_count, 10);
    ck_assert_int_eq(new_block->header->nBits, 0);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_destroy_block1) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_transaction = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_transaction, 0);
    finalize_block(genesis_b);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_destroy_block2) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    // Shortcut of block creating
    uint8_t *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);

    block_header_shortcut block_header = {.version = 0, .nonce = 0, .nBits = 0, .time = get_current_unix_time()};
    memset(block_header.prev_block_header_hash, 0, 32);
    memset(block_header.merkle_root_hash, 0, 32);
    memcpy(block_header.prev_block_header_hash, get_genesis_block_hash(), 32);
    transaction **txns = malloc(sizeof(transaction *));
    txns[0] = new_t;
    transactions_shortcut txns_shortcut = {.txns = txns, .txn_count = 1};
    block_create_shortcut block_data = {.header = &block_header, .transaction_list = &txns_shortcut};

    block *block1 = (block *)malloc(sizeof(block));
    create_new_block_shortcut(&block_data, block1);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_destroy_block3) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    uint8_t *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);

    block_header_shortcut block_header = {.version = 0, .nonce = 0, .nBits = 0, .time = get_current_unix_time()};
    memset(block_header.prev_block_header_hash, 0, 32);
    memset(block_header.merkle_root_hash, 0, 32);
    memcpy(block_header.prev_block_header_hash, get_genesis_block_hash(), 32);
    transaction **txns = malloc(sizeof(transaction *));
    txns[0] = new_t;
    transactions_shortcut txns_shortcut = {.txns = txns, .txn_count = 1};
    block_create_shortcut block_data = {.header = &block_header, .transaction_list = &txns_shortcut};

    block *block1 = (block *)malloc(sizeof(block));
    create_new_block_shortcut(&block_data, block1);
    finalize_block(block1);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_append_prev_block) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    block *new_block = create_an_empty_block(10);
    append_prev_block(genesis_b, new_block);

    uint8_t *expected_hash = hash_block_header(genesis_b->header);
    ck_assert_int_eq(memcmp(new_block->header->prev_block_header_hash, expected_hash, 32), 0);
    free(expected_hash);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_get_block_by_hash) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    uint8_t *block_header_hash = hash_block_header(genesis_b->header);
    block *retrieved_block = get_block_by_hash(block_header_hash);
    free(block_header_hash);

    ck_assert_int_eq(retrieved_block->txn_count, genesis_b->txn_count);
    ck_assert_ptr_nonnull(retrieved_block->header);
    ck_assert_ptr_nonnull(retrieved_block->txns);
    block_header *retrieved_header = retrieved_block->header;
    block_header *original_header = genesis_b->header;
    ck_assert_int_eq(retrieved_header->version, original_header->version);
    ck_assert_int_eq(retrieved_header->nBits, original_header->nBits);
    ck_assert_int_eq(retrieved_header->nonce, original_header->nonce);
    ck_assert_int_eq(retrieved_header->time, original_header->time);
    ck_assert_int_eq(memcmp(retrieved_header->prev_block_header_hash, original_header->prev_block_header_hash, 32), 0);
    ck_assert_int_eq(memcmp(retrieved_header->merkle_root_hash, original_header->merkle_root_hash, 32), 0);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_get_genesis_block_hash) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    uint8_t *computed = hash_block_header(genesis_b->header);
    ck_assert_int_eq(memcmp(computed, get_genesis_block_hash(), 32), 0);
    free(computed);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_hash_block_header) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    uint8_t *computed = hash_block_header(genesis_b->header);
    ck_assert_int_eq(memcmp(computed, get_genesis_block_hash(), 32), 0);
    free(computed);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_add_block_by_shortcut_and_finalize) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_append_transaction_into_block) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    ck_assert_ptr_nonnull(genesis_b->txns);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

START_TEST(test_verify_block_chain) {
    // Init
    initialize_mysql_system("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);
    block *genesis_b = initialize_block_system(false);
    append_transaction_into_block(genesis_b, genesis_t, 0);
    finalize_block(genesis_b);

    uint8_t *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);

    block_header_shortcut block_header = {.version = 0, .nonce = 0, .nBits = 0, .time = get_current_unix_time()};
    memset(block_header.prev_block_header_hash, 0, 32);
    memset(block_header.merkle_root_hash, 0, 32);
    memcpy(block_header.prev_block_header_hash, get_genesis_block_hash(), 32);
    transaction **txns = malloc(sizeof(transaction *));
    txns[0] = new_t;
    transactions_shortcut txns_shortcut = {.txns = txns, .txn_count = 1};
    block_create_shortcut block_data = {.header = &block_header, .transaction_list = &txns_shortcut};

    block *block1 = (block *)malloc(sizeof(block));
    create_new_block_shortcut(&block_data, block1);
    finalize_block(block1);

    verify_block_chain(block1);

    // Destroy.
    destroy_block_system("test");
    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}
END_TEST

Suite *transaction_suite(void) {
    Suite *s;
    s = suite_create("Block");

    TCase *tc_block_system_create = tcase_create("tc_block_system_create");
    tcase_add_test(tc_block_system_create, test_block_system_init_and_destroy);
    suite_add_tcase(s, tc_block_system_create);

    TCase *tc_create_empty_block = tcase_create("tc_create_empty_block");
    tcase_add_test(tc_create_empty_block, test_create_empty_block);
    suite_add_tcase(s, tc_create_empty_block);

    TCase *tc_destroy_block1 = tcase_create("tc_destroy_block1");
    tcase_add_test(tc_destroy_block1, test_destroy_block1);
    suite_add_tcase(s, tc_destroy_block1);

    TCase *tc_destroy_block2 = tcase_create("tc_destroy_block2");
    tcase_add_test(tc_destroy_block2, test_destroy_block2);
    suite_add_tcase(s, tc_destroy_block2);

    TCase *tc_destroy_block3 = tcase_create("tc_destroy_block3");
    tcase_add_test(tc_destroy_block3, test_destroy_block3);
    suite_add_tcase(s, tc_destroy_block3);

    TCase *tc_append_prev_block = tcase_create("tc_append_prev_block");
    tcase_add_test(tc_append_prev_block, test_append_prev_block);
    suite_add_tcase(s, tc_append_prev_block);

    TCase *tc_get_block_by_hash = tcase_create("tc_get_block_by_hash");
    tcase_add_test(tc_get_block_by_hash, test_get_block_by_hash);
    suite_add_tcase(s, tc_get_block_by_hash);

    TCase *tc_get_genesis_block_hash = tcase_create("tc_get_genesis_block_hash");
    tcase_add_test(tc_get_genesis_block_hash, test_get_genesis_block_hash);
    suite_add_tcase(s, tc_get_genesis_block_hash);

    TCase *tc_hash_block_header = tcase_create("tc_hash_block_header");
    tcase_add_test(tc_hash_block_header, test_hash_block_header);
    suite_add_tcase(s, tc_hash_block_header);

    TCase *tc_add_block_by_shortcut_and_finalize = tcase_create("tc_add_block_by_shortcut_and_finalize");
    tcase_add_test(tc_add_block_by_shortcut_and_finalize, test_add_block_by_shortcut_and_finalize);
    suite_add_tcase(s, tc_add_block_by_shortcut_and_finalize);

    TCase *tc_append_transaction_into_block = tcase_create("tc_append_transaction_into_block");
    tcase_add_test(tc_append_transaction_into_block, test_append_transaction_into_block);
    suite_add_tcase(s, tc_append_transaction_into_block);

    TCase *tc_verify_block_chain = tcase_create("tc_verify_block_chain");
    tcase_add_test(tc_verify_block_chain, test_verify_block_chain);
    suite_add_tcase(s, tc_verify_block_chain);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;
    s = transaction_suite();
    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
