#include "../src/model/block/block.h"

#include <check.h>
#include <stdlib.h>

#include "utils/constants.h"
#include "utils/sys_utils.h"

transaction* test_create_transaction_foo(transaction* previous_transaction, char* previous_private_key, unsigned char* current_private_key);


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
    destroy_block_system();
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
    destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_destroy_block1){
    /**
     * destroy an empty block that does not be added into the system
     */
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    block* block1 = create_an_empty_block(10);
    char* hash = hash_block_header(block1->header);
    destroy_block(block1);

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_destroy_block2){
    /**
     * Test create block but not finalize into the system
     */
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    //Shortcut of block creating
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {.previous_output_idx = 0,
                                               .previous_txid = previous_transaction_id,
                                               .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS,
                                                 .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {
        .num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction * new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);

//    unsigned char* current_private_key=NULL;
//    printf("-------%s---------------\n",current_private_key);
//    transaction* new_t = test_create_transaction_foo(genesis_t,get_genesis_transaction_private_key(),current_private_key);
//    printf("-------%s---------------\n",current_private_key);

    block_header_shortcut block_header={
        .prev_block_header_hash=NULL,
        .version=0,
        .nonce=0,
        .nBits=0,
        .merkle_root_hash=NULL,
        .time=get_current_unix_time()
    };
    memcpy(block_header.prev_block_header_hash,get_genesis_block_hash(),65);
    transaction **txns= malloc(sizeof(txns));
    txns[0]=new_t;
    transactions_shortcut txns_shortcut={
        .txns=txns,
        .txn_count=1
    };
    block_create_shortcut block_data={
        .header=&block_header,
        .transaction_list=&txns_shortcut
    };

    block* block1= (block *)malloc(sizeof(block));
    create_new_block_shortcut(&block_data, block1);
    char* hash = hash_block_header(block1->header);
    //destroy_block(block1);
    //ck_assert_ptr_null(get_block_by_hash(hash));

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_destroy_block3){
    /**
     * Test create a block and add into the system and then destroy it
     */
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction * genesis_t = initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    //Shortcut of the block
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {.previous_output_idx = 0,
                                               .previous_txid = previous_transaction_id,
                                               .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS,
                                                 .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {
        .num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction * new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);
    block_header_shortcut block_header={
        .prev_block_header_hash=NULL,
        .version=0,
        .nonce=0,
        .nBits=0,
        .merkle_root_hash=NULL,
        .time=get_current_unix_time()
    };
    memcpy(block_header.prev_block_header_hash,get_genesis_block_hash(),65);
    transaction **txns= malloc(sizeof(txns));
    txns[0]=new_t;
    transactions_shortcut txns_shortcut={
        .txns=txns,
        .txn_count=1
    };
    block_create_shortcut block_data={
        .header=&block_header,
        .transaction_list=&txns_shortcut
    };

    block* block1= (block *)malloc(sizeof(block));
    create_new_block_shortcut(&block_data, block1);
    finalize_block(block1);
    char* hash = hash_block_header(block1->header);
    //destroy_block(block1);
    //ck_assert_ptr_null(get_block_by_hash(hash));

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_append_prev_block){
    /**
     * Test append prev block
     */
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    block* new_block = create_an_empty_block(10);
    append_prev_block(genesis_b,new_block);
    ck_assert_str_eq(new_block->header->prev_block_header_hash, hash_block_header(genesis_b->header));

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

    ck_assert_ptr_eq(genesis_b, get_block_by_hash(hash_block_header(genesis_b->header)));

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_get_genesis_block_hash){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    ck_assert_str_eq(hash_block_header(genesis_b->header), get_genesis_block_hash());

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_hash_block_header){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    ck_assert_str_eq(hash_block_header(genesis_b->header), get_genesis_block_hash());

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_add_block_by_shortcut_and_finalize){
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

START_TEST(test_append_transaction_into_block){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction * genesis_t = initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    block* new_block = create_an_empty_block(10);
//    unsigned char* current_private_key = NULL;
//    test_create_transaction_foo(genesis_t,get_genesis_transaction_private_key(),current_private_key);
//    printf("---------%s\n",current_private_key);

    //Destroy.
    //TODO:destroy block
    //destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_verify_block_chain){
    //Init
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();
    block* genesis_b = initialize_block_system();

    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {.previous_output_idx = 0,
                                               .previous_txid = previous_transaction_id,
                                               .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS,
                                                 .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {
        .num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction * new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);
    block_header_shortcut block_header={
        .prev_block_header_hash=NULL,
        .version=0,
        .nonce=0,
        .nBits=0,
        .merkle_root_hash=NULL,
        .time=get_current_unix_time()
    };
    memcpy(block_header.prev_block_header_hash,get_genesis_block_hash(),65);
    transaction **txns= malloc(sizeof(txns));
    txns[0]=new_t;
    transactions_shortcut txns_shortcut={
        .txns=txns,
        .txn_count=1
    };
    block_create_shortcut block_data={
        .header=&block_header,
        .transaction_list=&txns_shortcut
    };

    block* block1= (block *)malloc(sizeof(block));
    create_new_block_shortcut(&block_data, block1);
    finalize_block(block1);

    verify_block_chain(block1);

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

    /* tc_destroy_block3 test case */
    TCase *tc_destroy_block3;
    tc_destroy_block3 = tcase_create("tc_destroy_block3");
    tcase_add_test(tc_destroy_block3, test_destroy_block3);
    suite_add_tcase(s, tc_destroy_block3);

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

    /* tc_get_genesis_block_hash test case */
    TCase *tc_get_genesis_block_hash;
    tc_get_genesis_block_hash = tcase_create("tc_get_genesis_block_hash");
    tcase_add_test(tc_get_genesis_block_hash, test_get_genesis_block_hash);
    suite_add_tcase(s, tc_get_genesis_block_hash);

    /* tc_hash_block_header test case */
    TCase *tc_hash_block_header;
    tc_hash_block_header = tcase_create("tc_hash_block_header");
    tcase_add_test(tc_hash_block_header, test_hash_block_header);
    suite_add_tcase(s, tc_hash_block_header);

    /* tc_add_block_by_shortcut_and_finalize test case */
    TCase *tc_add_block_by_shortcut_and_finalize;
    tc_add_block_by_shortcut_and_finalize = tcase_create("tc_add_block_by_shortcut_and_finalize");
    tcase_add_test(tc_add_block_by_shortcut_and_finalize, test_add_block_by_shortcut_and_finalize);
    suite_add_tcase(s, tc_add_block_by_shortcut_and_finalize);

    /* tc_append_transaction_into_block test case */
    TCase *tc_append_transaction_into_block;
    tc_append_transaction_into_block = tcase_create(tc_append_transaction_into_block);
    tcase_add_test(tc_append_transaction_into_block, test_append_transaction_into_block);
    suite_add_tcase(s, tc_append_transaction_into_block);

    /* tc_verify_block_chain test case */
    TCase *tc_verify_block_chain;
    tc_verify_block_chain = tcase_create(tc_verify_block_chain);
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

transaction* test_create_transaction_foo(transaction* previous_transaction, char* previous_private_key, unsigned char* current_private_key){
    char* previous_transaction_id = get_transaction_txid(previous_transaction);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 2, .previous_txid = previous_transaction_id, .private_key = previous_private_key};
    unsigned char* new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t = (transaction *)malloc(sizeof(transaction));
    create_new_transaction_shortcut(&create_data, new_t);
    finalize_transaction(new_t);
    *current_private_key = *new_private_key;
    return new_t;
}
