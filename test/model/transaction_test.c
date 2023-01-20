#include "../src/model/transaction/transaction.h"

#include <check.h>
#include <stdlib.h>

#include "../src/utils/constants.h"

START_TEST(test_transaction_system_create) {
    // Init.
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();
    // Start testing whether the genesis transaction's data is true.
    ck_assert_ptr_nonnull(genesis_t);
    ck_assert_int_eq(genesis_t->tx_in_count, 1);
    ck_assert_int_eq(genesis_t->tx_out_count, 1);
    ck_assert_ptr_nonnull(genesis_t->tx_ins);
    ck_assert_ptr_nonnull(genesis_t->tx_outs);
    ck_assert_str_eq(genesis_t->tx_ins[0].signature_script, "");
    ck_assert_int_eq(genesis_t->tx_ins[0].sequence, 1);
    ck_assert_int_eq(genesis_t->tx_ins[0].script_bytes, 0);
    ck_assert_int_eq(genesis_t->tx_outs[0].value, TOTAL_NUMBER_OF_COINS);
    ck_assert_ptr_nonnull(genesis_t->tx_outs->pk_script);
    ck_assert_int_eq(genesis_t->tx_outs[0].pk_script_bytes, 64);
    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_genesis_transaction_private_key) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    // Start testing whether the genesis transaction's private key is null.
    char *genesis_private_key = get_genesis_transaction_private_key();
    ck_assert_ptr_nonnull(genesis_private_key);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_genesis_transaction_public_key) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    // Start testing whether the genesis transaction's public key is null.
    char *genesis_pub_key = get_genesis_transaction_public_key();
    ck_assert_ptr_nonnull(genesis_pub_key);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_print_all_transaction) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_print_utxo) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();

    print_utxo();

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut1) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): false.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: false.
     * !append_new_transaction_input(ret_tx, input, i): false.
     * !append_new_transaction_output(ret_tx, output, i): false.
     * input_sum != output_sum: false
     */
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t1 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(create_new_transaction_shortcut(&create_data, new_t1), "Assert create new transaction successfully, but receive returning false!");
    ck_assert_msg(finalize_transaction(new_t1), "Assert create new transaction successfully, but receive returning false!");

    //Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut2){
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): true.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: false.
     * !append_new_transaction_input(ret_tx, input, i): false.
     * !append_new_transaction_output(ret_tx, output, i): false.
     * input_sum != output_sum: false.
     */
    char* previous_transaction_id = get_transaction_txid(genesis_t);
    previous_transaction_id[0]='a';
    transaction_create_shortcut_input input2 = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key2 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key2 = get_a_new_public_key((char *)new_private_key2);
    transaction_create_shortcut_output output2 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key2->data};
    transaction_create_shortcut create_data2 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output2, .inputs = &input2};
    transaction *new_t2 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(!create_new_transaction_shortcut(&create_data2, new_t2), "Assert create new transaction fail, but receive returning true!");
    //TODO: finalize
    //ck_assert_msg(!finalize_transaction(new_t2), "Assert create new transaction fail, but receive returning true!");

    //Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut3){
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): false.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: true.
     * !append_new_transaction_input(ret_tx, input, i): false.
     * !append_new_transaction_output(ret_tx, output, i): false.
     * input_sum != output_sum: false
     */
    char* previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input3 = {
        .previous_output_idx = 2, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key3 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key3 = get_a_new_public_key((char *)new_private_key3);
    transaction_create_shortcut_output output3 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key3->data};
    transaction_create_shortcut create_data3 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output3, .inputs = &input3};
    transaction *new_t3 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(!create_new_transaction_shortcut(&create_data3, new_t3), "Assert create new transaction successfully, but receive returning false!");
    //TODO: finalize
    //ck_assert_msg(!finalize_transaction(new_t3), "Assert create new transaction successfully, but receive returning false!");

    //Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(t3){

}
END_TEST

START_TEST(t2){

}
END_TEST


START_TEST(test_get_transaction_txid) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();

    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t1 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(create_new_transaction_shortcut(&create_data, new_t1), "Assert create new transaction successfully, but receive returning false!");
    ck_assert_msg(finalize_transaction(new_t1), "Assert create new transaction successfully, but receive returning false!");
    ck_assert_str_eq(hash_struct_in_hex(new_t1, sizeof(transaction)), get_transaction_txid(new_t1));

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_get_transaction_by_txid) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();

    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t1 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(create_new_transaction_shortcut(&create_data, new_t1), "Assert create new transaction successfully, but receive returning false!");
    ck_assert_msg(finalize_transaction(new_t1), "Assert create new transaction successfully, but receive returning false!");
    ck_assert_ptr_eq(get_transaction_by_txid(hash_struct_in_hex(new_t1, sizeof(transaction))), new_t1);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_destroy_transaction) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction* genesis_t = initialize_transaction_system();

    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key = get_a_new_private_key();
    secp256k1_pubkey *new_public_key = get_a_new_public_key((char *)new_private_key);
    transaction_create_shortcut_output output = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key->data};
    transaction_create_shortcut create_data = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output, .inputs = &input};
    transaction *new_t1 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(create_new_transaction_shortcut(&create_data, new_t1), "Assert create new transaction successfully, but receive returning false!");
    ck_assert_msg(finalize_transaction(new_t1), "Assert create new transaction successfully, but receive returning false!");
    //destroy_transaction(new_t1);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

Suite *transaction_suite(void) {
    Suite *s;
    s = suite_create("Transaction");

    /* system_create test case */
    TCase *tc_system_create;
    tc_system_create = tcase_create("tc_system_create");
    tcase_add_test(tc_system_create, test_transaction_system_create);
    suite_add_tcase(s, tc_system_create);

    /* get_genesis_t_private_key test case */
    TCase *tc_get_genesis_t_private_key;
    tc_get_genesis_t_private_key = tcase_create("tc_get_genesis_t_private_key");
    tcase_add_test(tc_get_genesis_t_private_key, test_genesis_transaction_private_key);
    suite_add_tcase(s, tc_get_genesis_t_private_key);

    /* tc_get_genesis_t_pub_key test case */
    TCase *tc_get_genesis_t_pub_key;
    tc_get_genesis_t_pub_key = tcase_create("tc_get_genesis_t_pub_key");
    tcase_add_test(tc_get_genesis_t_pub_key, test_genesis_transaction_public_key);
    suite_add_tcase(s, tc_get_genesis_t_pub_key);

    /* tc_print_transactions test case */
    TCase *tc_print_transactions;
    tc_print_transactions = tcase_create("tc_print_transactions");
    tcase_add_test(tc_print_transactions, test_print_all_transaction);
    suite_add_tcase(s, tc_print_transactions);

    /* tc_print_utxo test case */
    TCase *tc_print_utxo;
    tc_print_utxo = tcase_create("tc_print_utxo");
    tcase_add_test(tc_print_utxo, test_print_utxo);
    suite_add_tcase(s, tc_print_utxo);

    /* tc_get_transaction_txid test case */
    TCase *tc_get_transaction_txid;
    tc_get_transaction_txid = tcase_create("tc_get_transaction_txid");
    tcase_add_test(tc_get_transaction_txid, test_get_transaction_txid);
    suite_add_tcase(s, tc_get_transaction_txid);

    /* tc_get_transaction_by_txid test case */
    TCase *tc_get_transaction_by_txid;
    tc_get_transaction_by_txid = tcase_create("tc_get_transaction_by_txid");
    tcase_add_test(tc_get_transaction_by_txid, test_get_transaction_by_txid);
    suite_add_tcase(s, tc_get_transaction_by_txid);

    /* tc_destroy_transaction test case */
    TCase *tc_destroy_transaction;
    tc_destroy_transaction = tcase_create("tc_destroy_transaction");
    tcase_add_test(tc_destroy_transaction, test_destroy_transaction);
    suite_add_tcase(s, tc_destroy_transaction);

    /* tc_create_new_transaction_shortcut1 test case */
    TCase *tc_create_new_transaction_shortcut1;
    tc_create_new_transaction_shortcut1 = tcase_create("tc_create_new_transaction_shortcut1");
    tcase_add_test(tc_create_new_transaction_shortcut1, test_create_new_transaction_shortcut1);
    suite_add_tcase(s, tc_create_new_transaction_shortcut1);

    /* tc_create_new_transaction_shortcut2 test case */
    TCase *tc_create_new_transaction_shortcut2;
    tc_create_new_transaction_shortcut2 = tcase_create("tc_create_new_transaction_shortcut2");
    tcase_add_test(tc_create_new_transaction_shortcut2, test_create_new_transaction_shortcut2);
    suite_add_tcase(s, tc_create_new_transaction_shortcut2);

    /* tc_create_new_transaction_shortcut3 test case */
    TCase *tc_create_new_transaction_shortcut3;
    tc_create_new_transaction_shortcut3 = tcase_create("tc_create_new_transaction_shortcut3");
    tcase_add_test(tc_create_new_transaction_shortcut3, test_create_new_transaction_shortcut3);
    suite_add_tcase(s, tc_create_new_transaction_shortcut3);

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