#include "../src/model/transaction/transaction.h"

#include <check.h>
#include <stdlib.h>

#include "../src/model/transaction/transaction_persistence.h"
#include "../src/utils/constants.h"
#include "../src/utils/mysql_util.h"

START_TEST(test_transaction_system_create) {
    printf("%s\n", "test_transaction_system_create start!");

    // Init.
    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();
    // Start testing whether the genesis transaction's data is true.
    ck_assert_ptr_nonnull(genesis_t);
    ck_assert_int_eq(genesis_t->tx_in_count, 1);
    ck_assert_int_eq(genesis_t->tx_out_count, 1);
    ck_assert_ptr_nonnull(genesis_t->tx_ins);
    ck_assert_ptr_nonnull(genesis_t->tx_outs);
    ck_assert_str_eq(genesis_t->tx_ins[0].signature_script, "A");
    ck_assert_int_eq(genesis_t->tx_ins[0].sequence, 1);
    ck_assert_int_eq(genesis_t->tx_ins[0].script_bytes, 1);
    ck_assert_int_eq(genesis_t->tx_outs[0].value, TOTAL_NUMBER_OF_COINS);
    ck_assert_ptr_nonnull(genesis_t->tx_outs->pk_script);
    ck_assert_int_eq(genesis_t->tx_outs[0].pk_script_bytes, 64);
    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_genesis_transaction_private_key) {
    printf("%s\n", "test_genesis_transaction_private_key start!");

    initialize_mysql_system();
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
    printf("%s\n", "test_genesis_transaction_public_key start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    // Start testing whether the genesis transaction's public key is null.
    secp256k1_pubkey *genesis_pub_key = get_genesis_transaction_public_key();
    ck_assert_ptr_nonnull(genesis_pub_key);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_print_utxo) {
    printf("%s\n", "test_print_utxo start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();

    print_utxo();

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut1) {
    printf("%s\n", "test_create_new_transaction_shortcut1 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): false.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: false.
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

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut2) {
    printf("%s\n", "test_create_new_transaction_shortcut2 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): true.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: false.
     * input_sum != output_sum: false.
     */
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    previous_transaction_id[0] = 'a';
    transaction_create_shortcut_input input2 = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key2 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key2 = get_a_new_public_key((char *)new_private_key2);
    transaction_create_shortcut_output output2 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key2->data};
    transaction_create_shortcut create_data2 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output2, .inputs = &input2};
    transaction *new_t2 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(!create_new_transaction_shortcut(&create_data2, new_t2), "Assert create new transaction fail, but receive returning true!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut3) {
    printf("%s\n", "test_create_new_transaction_shortcut3 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): false.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: true.
     */
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input3 = {
        .previous_output_idx = 2, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key3 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key3 = get_a_new_public_key((char *)new_private_key3);
    transaction_create_shortcut_output output3 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key3->data};
    transaction_create_shortcut create_data3 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output3, .inputs = &input3};
    transaction *new_t3 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(!create_new_transaction_shortcut(&create_data3, new_t3), "Assert create new transaction fail, but receive returning true!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut4) {
    printf("%s\n", "test_create_new_transaction_shortcut4 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): false.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: false.
     * !append_new_transaction_input(ret_tx, input, i): true -> utxo false
     */
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input4 = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key4 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key4 = get_a_new_public_key((char *)new_private_key4);
    transaction_create_shortcut_output output4 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key4->data};
    transaction_create_shortcut create_data4 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output4, .inputs = &input4};
    transaction *new_t4 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(create_new_transaction_shortcut(&create_data4, new_t4), "Assert create new transaction fail, but receive returning true!");
    ck_assert_msg(finalize_transaction(new_t4), "Assert create new transaction fail, but receive returning pass!");

    previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input5 = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key5 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key5 = get_a_new_public_key((char *)new_private_key5);
    transaction_create_shortcut_output output5 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key5->data};
    transaction_create_shortcut create_data5 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output5, .inputs = &input5};
    transaction *new_t5 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(!create_new_transaction_shortcut(&create_data5, new_t5), "Assert create new transaction fail, but receive returning true!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_create_new_transaction_shortcut5) {
    printf("%s\n", "test_create_new_transaction_shortcut5 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * !g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid): false.
     * curr_input_data.previous_output_idx >= previous_tx->tx_out_count: false.
     * !append_new_transaction_input(ret_tx, input, i): true -> signature fail due to public key
     */
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input4 = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key4 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key4 = get_a_new_public_key((char *)new_private_key4);
    transaction_create_shortcut_output output4 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key4->data};
    transaction_create_shortcut create_data4 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output4, .inputs = &input4};
    transaction *new_t4 = (transaction *)malloc(sizeof(transaction));

    memcpy(genesis_t->tx_outs[0].pk_script, new_public_key4->data, 64);
    ck_assert_msg(!create_new_transaction_shortcut(&create_data4, new_t4), "Assert create new transaction fail, but receive returning true!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_get_transaction_txid) {
    printf("%s\n", "test_get_transaction_txid start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

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

    transaction *copied_tx = (transaction *)malloc(sizeof(transaction));
    memset(copied_tx, 0, sizeof(transaction));
    copied_tx->tx_out_count = new_t1->tx_out_count;
    copied_tx->tx_in_count = new_t1->tx_in_count;
    copied_tx->lock_time = new_t1->lock_time;
    copied_tx->version = new_t1->version;

    ck_assert_str_eq(hash_struct_in_hex(copied_tx, sizeof(transaction)), get_transaction_txid(new_t1));

    free(copied_tx);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_get_transaction_by_txid) {
    printf("%s\n", "test_get_transaction_by_txid start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

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

    transaction *copied_tx = (transaction *)malloc(sizeof(transaction));
    memset(copied_tx, 0, sizeof(transaction));
    copied_tx->tx_out_count = new_t1->tx_out_count;
    copied_tx->tx_in_count = new_t1->tx_in_count;
    copied_tx->lock_time = new_t1->lock_time;
    copied_tx->version = new_t1->version;

    transaction *retrieved_tx = get_transaction_by_txid(hash_struct_in_hex(copied_tx, sizeof(transaction)));
    ck_assert_int_eq(retrieved_tx->tx_in_count, new_t1->tx_in_count);
    ck_assert_int_eq(retrieved_tx->tx_out_count, new_t1->tx_out_count);
    ck_assert_int_eq(retrieved_tx->lock_time, new_t1->lock_time);
    ck_assert_int_eq(retrieved_tx->version, new_t1->version);

    free(copied_tx);

    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_verify_transaction1) {
    printf("%s\n", "test_verify_transaction1 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Condition:
     * 1. !verify_transaction_input(&t->tx_ins[i], true) : false
     * 2. input_sum != output_sum : false
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
    ck_assert_msg(verify_transaction(new_t1), "Assert verify the transaction successfully, but receiving false!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_verify_transaction2) {
    printf("%s\n", "test_verify_transaction2 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Condition:
     * 1. !verify_transaction_input(&t->tx_ins[i], true) : true -> hash failed
     * 2. input_sum != output_sum : false
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

    memcpy(new_t1->tx_ins[0].previous_outpoint.hash, get_transaction_txid(new_t1), 64);
    new_t1->tx_ins[0].previous_outpoint.hash[64] = '\0';
    ck_assert_msg(!verify_transaction(new_t1), "Assert verify the transaction fail, but receiving pass!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_verify_transaction3) {
    printf("%s\n", "test_verify_transaction3 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Condition:
     * 1. !verify_transaction_input(&t->tx_ins[i], true) : true -> output_idx >= previous_transaction->tx_out_count
     * 2. input_sum != output_sum : false
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

    new_t1->tx_ins[0].previous_outpoint.index = 2;
    ck_assert_msg(!verify_transaction(new_t1), "Assert verify the transaction fail, but receiving pass!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_verify_transaction4) {
    printf("%s\n", "test_verify_transaction4 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Condition:
     * 1. !verify_transaction_input(&t->tx_ins[i], true) : true -> public key wrong, signature verify fail
     * 2. input_sum != output_sum : false
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

    memcpy(genesis_t->tx_outs[0].pk_script, new_public_key->data, 64);
    ck_assert_msg(!verify_transaction(new_t1), "Assert verify the transaction fail, but receiving pass!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_verify_transaction5) {
    printf("%s\n", "test_verify_transaction5 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Condition:
     * 1. !verify_transaction_input(&t->tx_ins[i], true) : false
     * 2. input_sum != output_sum : true
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

    new_t1->tx_outs[0].value = TOTAL_NUMBER_OF_COINS - 1;
    ck_assert_msg(!verify_transaction(new_t1), "Assert verify the transaction fail, but receiving pass!");

    // Destroy.
    destroy_transaction_system();
    destroy_cryptography_system();
}
END_TEST

START_TEST(test_finalize_transaction2) {
    printf("%s\n", "test_finalize_transaction2 start!");

    initialize_mysql_system();
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system();

    /**
     * Test case:
     * 1. input_sum != output_sum : true
     */
    char *previous_transaction_id = get_transaction_txid(genesis_t);
    transaction_create_shortcut_input input4 = {
        .previous_output_idx = 0, .previous_txid = previous_transaction_id, .private_key = get_genesis_transaction_private_key()};
    unsigned char *new_private_key4 = get_a_new_private_key();
    secp256k1_pubkey *new_public_key4 = get_a_new_public_key((char *)new_private_key4);
    transaction_create_shortcut_output output4 = {.value = TOTAL_NUMBER_OF_COINS, .public_key = (char *)new_public_key4->data};
    transaction_create_shortcut create_data4 = {.num_of_inputs = 1, .num_of_outputs = 1, .outputs = &output4, .inputs = &input4};
    transaction *new_t4 = (transaction *)malloc(sizeof(transaction));

    ck_assert_msg(create_new_transaction_shortcut(&create_data4, new_t4), "Assert create new transaction fail, but receive returning true!");

    new_t4->tx_outs[0].value = TOTAL_NUMBER_OF_COINS - 1;
    ck_assert_msg(!finalize_transaction(new_t4), "Assert create new transaction fail, but receive returning pass!");

    // Destroy.
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

    /* tc_create_new_transaction_shortcut4 test case */
    TCase *tc_create_new_transaction_shortcut4;
    tc_create_new_transaction_shortcut4 = tcase_create("tc_create_new_transaction_shortcut4");
    tcase_add_test(tc_create_new_transaction_shortcut4, test_create_new_transaction_shortcut4);
    suite_add_tcase(s, tc_create_new_transaction_shortcut4);

    /* tc_create_new_transaction_shortcut5 test case */
    TCase *tc_create_new_transaction_shortcut5;
    tc_create_new_transaction_shortcut5 = tcase_create("tc_create_new_transaction_shortcut5");
    tcase_add_test(tc_create_new_transaction_shortcut5, test_create_new_transaction_shortcut5);
    suite_add_tcase(s, tc_create_new_transaction_shortcut5);

    /* tc_verify_transaction1 test case */
    TCase *tc_verify_transaction1;
    tc_verify_transaction1 = tcase_create("tc_verify_transaction1");
    tcase_add_test(tc_verify_transaction1, test_verify_transaction1);
    suite_add_tcase(s, tc_verify_transaction1);

    /* tc_verify_transaction2 test case */
    TCase *tc_verify_transaction2;
    tc_verify_transaction2 = tcase_create("tc_verify_transaction2");
    tcase_add_test(tc_verify_transaction2, test_verify_transaction2);
    suite_add_tcase(s, tc_verify_transaction2);

    /* tc_verify_transaction3 test case */
    TCase *tc_verify_transaction3;
    tc_verify_transaction3 = tcase_create("tc_verify_transaction3");
    tcase_add_test(tc_verify_transaction3, test_verify_transaction3);
    suite_add_tcase(s, tc_verify_transaction3);

    /* tc_verify_transaction4 test case */
    TCase *tc_verify_transaction4;
    tc_verify_transaction4 = tcase_create("tc_verify_transaction4");
    tcase_add_test(tc_verify_transaction4, test_verify_transaction4);
    suite_add_tcase(s, tc_verify_transaction4);

    /* tc_verify_transaction5 test case */
    TCase *tc_verify_transaction5;
    tc_verify_transaction5 = tcase_create("tc_verify_transaction5");
    tcase_add_test(tc_verify_transaction5, test_verify_transaction5);
    suite_add_tcase(s, tc_verify_transaction5);

    /* tc_finalize_transaction2 test case */
    TCase *tc_finalize_transaction2;
    tc_finalize_transaction2 = tcase_create("tc_finalize_transaction2");
    tcase_add_test(tc_finalize_transaction2, test_finalize_transaction2);
    suite_add_tcase(s, tc_finalize_transaction2);

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