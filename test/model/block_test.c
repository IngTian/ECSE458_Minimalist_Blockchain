#include <check.h>
#include <stdlib.h>
#include "../src/model/block/block.h"

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
    destroy_block_system();
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

//block *create_an_empty_block(unsigned int);
//void destroy_block(block *);
//bool append_prev_block(block *prev_block, block *cur_block);
//bool finalize_block(block *);
//block *get_block_by_hash(char *);
//bool append_transaction_into_block(block *, transaction *, unsigned int input_idx);
//bool verify_block_chain(block *);
//char *get_genesis_block_hash();
//void print_block_chain(block *);