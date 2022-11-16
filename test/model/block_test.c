#include "../src/model/block/block.h"

#include <check.h>
#include <stdlib.h>

#include "utils/constants.h"

START_TEST(test_block_system_init_and_destroy) {
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    ck_assert_ptr_nonnull(genesis_b);
    ck_assert_ptr_nonnull(genesis_b->header);
    ck_assert_int_eq(genesis_b->header->nBits,0);
    ck_assert_int_eq(genesis_b->header->nonce,0);
    ck_assert_ptr_nonnull(genesis_b->txns);
    ck_assert_int_eq(genesis_b->txn_count,0);

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_empty_block){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    block* new_block = create_an_empty_block(10);
    ck_assert_ptr_nonnull(new_block);
    ck_assert_int_eq(new_block->header->version,0);
    ck_assert_int_eq(new_block->header->nonce,0);
    ck_assert_int_eq(new_block->txn_count,0);
    ck_assert_int_eq(new_block->header->nBits,0);

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_destroy_block1){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    block* block1 = create_an_empty_block(10);
    destroy_block(block1);
    //ck_assert_ptr_null(block1);

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_destroy_block2){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    //TODO: use shortcut to add transactions
    block* block1;
    destroy_block(block1);
    //ck_assert_ptr_null(block1);

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_append_prev_block){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    block* new_block = create_an_empty_block(10);
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t1 = (transaction *)malloc(sizeof(transaction));
    append_transaction_into_block(new_block, new_t1,0);


    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_get_block_by_hash){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST




Suite *transaction_suite(void) {
    Suite *s;
    s = suite_create("Block");

    /* tc_block_system_create test case */
    TCase *tc_block_system_create;
    tc_block_system_create = tcase_create("tc_block_system_create");
    tcase_add_test(tc_block_system_create, test_block_system_init_and_destroy);
    suite_add_tcase(s, tc_block_system_create);

    /* tc_create_empty_block test case */
    TCase *tc_create_empty_block;
    tc_create_empty_block = tcase_create("tc_create_empty_block");
    tcase_add_test(tc_create_empty_block, test_create_empty_block);
    suite_add_tcase(s, tc_create_empty_block);

    /* tc_destroy_block1 test case */
    TCase *tc_destroy_block1;
    tc_destroy_block1 = tcase_create("tc_destroy_block1");
    tcase_add_test(tc_destroy_block1, test_destroy_block1);
    suite_add_tcase(s, tc_destroy_block1);

    /* tc_destroy_block2 test case */
    TCase *tc_destroy_block2;
    tc_destroy_block2 = tcase_create("tc_destroy_block2");
    tcase_add_test(tc_destroy_block2, test_destroy_block2);
    suite_add_tcase(s, tc_destroy_block2);

    /* tc_append_prev_block test case */
    TCase *tc_append_prev_block;
    tc_append_prev_block = tcase_create("tc_append_prev_block");
    tcase_add_test(tc_append_prev_block, test_append_prev_block);
    suite_add_tcase(s, tc_append_prev_block);

    /* tc_get_block_by_hash test case */
    TCase *tc_get_block_by_hash;
    tc_get_block_by_hash = tcase_create("tc_get_block_by_hash");
    tcase_add_test(tc_get_block_by_hash, test_get_block_by_hash);
    suite_add_tcase(s, tc_get_block_by_hash);


    //TODO: taske name
    /* tc_append_transaction_into_block test case */
    TCase *tc_append_transaction_into_block;
    tc_append_transaction_into_block = tcase_create("tc_append_transaction_into_block");
    tcase_add_test(tc_append_transaction_into_block, test_get_block_by_hash);
    suite_add_tcase(s, tc_append_transaction_into_block);

    /* tc_verify_block_chain test case */
    TCase *tc_verify_block_chain;
    tc_verify_block_chain = tcase_create("tc_verify_block_chain");
    tcase_add_test(tc_verify_block_chain, test_get_block_by_hash);
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

//void destroy_block(block *);
//bool append_prev_block(block *prev_block, block *cur_block);
//bool finalize_block(block *);
//block *get_block_by_hash(char *);
//bool append_transaction_into_block(block *, transaction *, unsigned int input_idx);
//bool verify_block_chain(block *);
//char *get_genesis_block_hash();
//void print_block_chain(block *);