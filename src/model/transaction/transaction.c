#include "transaction.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/constants.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "transaction"

static unsigned int g_total_number_of_transactions;  // The total number of transactions in the system.
static GHashTable *g_global_transaction_table;       // The global transaction table, mapping TXID to transaction.
static GHashTable *g_utxo;  // Unspent Transaction Output. mapping each transaction output to its value left.
char *g_genesis_private_key;
secp256k1_pubkey *g_genesis_public_key;

char *hash_transaction_outpoint(transaction_outpoint *);
char *hash_transaction_output(transaction_output *);
unsigned int get_total_number_of_transactions();
transaction *create_an_empty_transaction(unsigned int, unsigned int);
bool append_new_transaction_input(transaction *, transaction_input, unsigned int);
bool append_new_transaction_output(transaction *, transaction_output, unsigned int);

/*
 * -----------------------------------------------------------
 * Helper Methods
 * -----------------------------------------------------------
 */
/**
 * Verify that the transaction input is valid, i.e.,
 * whoever signs the input has the right to use the
 * specified output.
 * @param i A transaction input.
 * @return True for valid, false otherwise
 * @author Ing Tian
 */
bool verify_transaction_input(transaction_input *i) {
    transaction_outpoint outpoint = i->previous_outpoint;
    char *transaction_hash = outpoint.hash;
    unsigned int output_idx = outpoint.index;

    if (!g_hash_table_contains(g_global_transaction_table, transaction_hash)) {
        return false;
    }

    transaction *previous_transaction = g_hash_table_lookup(g_global_transaction_table, transaction_hash);

    if (output_idx >= previous_transaction->tx_out_count) {
        return false;
    }

    transaction_output previous_transaction_output = previous_transaction->tx_outs[output_idx];
    char *hash_msg = hash_transaction_output(&previous_transaction_output);
    secp256k1_pubkey pubkey;
    secp256k1_ecdsa_signature signature;
    memcpy(pubkey.data, previous_transaction_output.pk_script, 64);
    memcpy(signature.data, i->signature_script, 64);

    char *utxo_key = hash_transaction_outpoint(&outpoint);
    if (!g_hash_table_contains(g_utxo, utxo_key)) {
        general_log(LOG_SCOPE, LOG_ERROR, "UTXO is over spent.");
        return false;
    }
    free(utxo_key);

    bool result = verify(&pubkey, (unsigned char *)hash_msg, &signature);
    free(hash_msg);

    return result;
}

/**
 * Free a transaction input.
 * @param input
 * @author Ing Tian
 */
void free_transaction_input(transaction_input *input) {
    free(input->signature_script);
    free(input);
}

/**
 * Free a transaction output.
 * @param output
 * @author Ing Tian
 */
static void free_transaction_output(transaction_output *output) {
    free(output->pk_script);
    free(output);
}

void print_transaction(void *transaction_id, void *tx, void *user_data) {
    char *txid = (char *)transaction_id;
    transaction *t = (transaction *)tx;
    printf("TXID: %s VERSION: %d TX IN COUNT: %u TX OUT COUNT: %u LOCK: %u\n", txid, t->version, t->tx_in_count,
           t->tx_out_count, t->lock_time);
    printf("<Printing Inputs>\n");
    for (int i = 0; i < t->tx_in_count; i++) {
        transaction_input input = t->tx_ins[i];
        printf("SEQ: %u SIG: %s OUTPOINT HASH: %s OUTPOINT ID: %u\n", input.sequence,
               convert_char_hexadecimal(input.signature_script, 64), input.previous_outpoint.hash,
               input.previous_outpoint.index);
    }
    printf("<Printing Outputs>\n");
    for (int i = 0; i < t->tx_out_count; i++) {
        transaction_output output = t->tx_outs[i];
        char *pk_hex = convert_char_hexadecimal(output.pk_script, output.pk_script_bytes);
        printf("VAL: %ld PK(HEX): %s\n", output.value, pk_hex);
    }
    printf("\n");
}

void print_utxo_entry(void *h, void *v, void *user_data) {
    char *hash = (char *)h;
    long int *value = (long int *)v;
    printf("ID: %s VAL: %ld\n", convert_char_hexadecimal(hash, 32), *value);
    printf("\n");
}

void free_transaction_table_entry(void *txid, void *tx, void *user_data) {
    free(txid);
    destroy_transaction((transaction *)tx);
}

void free_utxo_table_entry(void *utxo_id, void *val, void *user_data) {
    free(utxo_id);
    free(val);
}

/*
 * -----------------------------------------------------------
 * Functionalities
 * -----------------------------------------------------------
 */
/**
 * Setup the transaction system, including setting up:
 * 1. The global transaction table.
 * 2. Unspent Transaction Output (UTXO)
 * @return The first transaction in the system, known as the genesis transaction.
 * @author Ing Tian
 */
transaction *initialize_transaction_system() {
    g_global_transaction_table = g_hash_table_new(g_str_hash, g_str_equal);
    g_utxo = g_hash_table_new(g_str_hash, g_str_equal);

    transaction *genesis_transaction = create_an_empty_transaction(1, 1);
    genesis_transaction->tx_ins[0].signature_script = "";
    genesis_transaction->tx_ins[0].sequence = 1;
    genesis_transaction->tx_ins[0].script_bytes = 0;
    genesis_transaction->tx_outs[0].value = TOTAL_NUMBER_OF_COINS;
    g_genesis_private_key = (char *)get_a_new_private_key();
    g_genesis_public_key = get_a_new_public_key(g_genesis_private_key);
    genesis_transaction->tx_outs->pk_script = (char *)malloc(64);
    memcpy(genesis_transaction->tx_outs->pk_script, g_genesis_public_key->data, 64);
    genesis_transaction->tx_outs[0].pk_script_bytes = 64;

    char *genesis_txid = get_transaction_txid(genesis_transaction);

    transaction_outpoint outpoint = {.index = 0};
    memcpy(outpoint.hash, genesis_txid, 64);
    char *outpoint_hash = hash_transaction_outpoint(&outpoint);
    long int *genesis_balance = (long int *)malloc(sizeof(long int));
    *genesis_balance = TOTAL_NUMBER_OF_COINS;

    g_hash_table_insert(g_utxo, outpoint_hash, genesis_balance);
    g_hash_table_insert(g_global_transaction_table, genesis_txid, genesis_transaction);

    general_log(LOG_SCOPE, LOG_INFO, "Initialized the transaction module. Genesis TXID: %s", genesis_txid);

    return genesis_transaction;
}

/**
 * Destroy the transaction system.
 * @author Ing Tian
 */
void destroy_transaction_system() {
    g_hash_table_foreach(g_global_transaction_table, free_transaction_table_entry, NULL);
    g_hash_table_foreach(g_utxo, free_utxo_table_entry, NULL);
    free(g_utxo);
    free(g_global_transaction_table);

    general_log(LOG_SCOPE, LOG_INFO, "Destroyed the transaction module.");
}

/**
 * Get the transaction_id of a transaction.
 * @param transaction
 * @return The hash of the transaction (32 bytes).
 * @author Ing Tian
 */
char *get_transaction_txid(transaction *t) { return hash_struct_in_hex(t, sizeof(transaction)); }

/**
 * Get the SHA256 hashcode of a transaction output.
 * @param output A transaction output.
 * @return The SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_transaction_output(transaction_output *output) {
    return hash_struct_in_hex(output, sizeof(transaction_output));
}

/**
 * Get the SHA256 hashcode of a transaction outpoint.
 * @param outpoint A transaction outpoint.
 * @return The SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_transaction_outpoint(transaction_outpoint *outpoint) {
    return hash_struct_in_hex(outpoint, sizeof(transaction_outpoint));
}

/**
 * Get the total number of transactions in the system.
 * @return The total number of transactions in the system.
 * @author Ing Tian
 */
unsigned int get_total_number_of_transactions() { return g_total_number_of_transactions; }

/**
 * Get the private key of the genesis transaction.
 * @return Private key of the genesis transaction.
 * @author Ing Tian
 */
char *get_genesis_transaction_private_key() { return g_genesis_private_key; }

/**
 * Get the public key of the genesis transaction.
 * @return Public key of the genesis transaction.
 * @author Ing Tian
 */
secp256k1_pubkey *get_genesis_transaction_public_key() { return g_genesis_public_key; }

/**
 * * Create an empty transaction.
 * @param num_of_inputs Number of inputs in the transaction.
 * @param num_of_outputs Number of outputs in the transaction.
 * @return An empty transaction.
 * @author Ing Tian
 */
transaction *create_an_empty_transaction(unsigned int num_of_inputs, unsigned int num_of_outputs) {
    transaction *t = (transaction *)malloc(sizeof(transaction));
    t->version = 1;
    t->tx_in_count = num_of_inputs;
    t->tx_ins = (transaction_input *)malloc(sizeof(transaction_input) * num_of_inputs);
    t->tx_out_count = num_of_outputs;
    t->tx_outs = (transaction_output *)malloc(sizeof(transaction_output) * num_of_outputs);
    t->lock_time = get_timestamp();
    return t;
}

/**
 * Destroy a transaction, free all of its memory space.
 * Notice that the transaction should not have been
 * registered in the system. A transaction registered
 * shall not be destroyed.
 * @param t A transaction
 * @author Ing Tian
 */
void destroy_transaction(transaction *t) {
    for (int i = 0; i < t->tx_in_count; i++) free_transaction_input(&t->tx_ins[i]);
    for (int i = 0; i < t->tx_out_count; i++) free_transaction_output(&t->tx_outs[i]);
    free(t);
}

/**
 * Append a new transaction input to a transaction.
 * @param t A transaction.
 * @param input A new input.
 * @return True for success, false otherwise.
 * @author Ing Tian
 */
bool append_new_transaction_input(transaction *t, transaction_input input, unsigned int input_idx) {
    if (!verify_transaction_input(&input)) return false;
    memcpy(&t->tx_ins[input_idx], &input, sizeof(transaction_input));
    return true;
}

/**
 * Append a new transaction output to a transaction.
 * @param t A transaction.
 * @param output A new output.
 * @return True for success, false otherwise.
 * @auhor Ing Tian
 */
bool append_new_transaction_output(transaction *t, transaction_output output, unsigned int output_idx) {
    if (t == NULL) return false;
    memcpy(&t->tx_outs[output_idx], &output, sizeof(transaction_output));
    return true;
}

/**
 * Finalize a transaction in the system; this action
 * will update the global transaction table and the
 * UTXO.
 * @param t A transaction.
 * @return True for success, false otherwise.
 * @author Ing Tian
 */
bool finalize_transaction(transaction *t) {
    // Check if the sum of input is equal
    // to the sum of outputs.
    long int input_sum = 0, output_sum = 0;

    for (int i = 0; i < t->tx_in_count; i++) {
        transaction_input input = t->tx_ins[i];
        char *previous_transaction_id = input.previous_outpoint.hash;
        unsigned int previous_output_id = input.previous_outpoint.index;
        transaction *previous_transaction = g_hash_table_lookup(g_global_transaction_table, previous_transaction_id);
        input_sum += previous_transaction->tx_outs[previous_output_id].value;
    }

    for (int i = 0; i < t->tx_out_count; i++) {
        output_sum += t->tx_outs[i].value;
    }

    if (input_sum != output_sum) {
        general_log(LOG_SCOPE, LOG_ERROR, "Input (%ld) does not equal output (%ld).", input_sum, output_sum);
        return false;
    }

    // Register this transaction in the system.
    char *txid = get_transaction_txid(t);
    g_hash_table_insert(g_global_transaction_table, txid, t);

    // Update UTXO.
    for (int i = 0; i < t->tx_in_count; i++) {
        char *outpoint_hash = hash_transaction_outpoint(&t->tx_ins[i].previous_outpoint);
        g_hash_table_remove(g_utxo, outpoint_hash);
        free(outpoint_hash);
    }

    for (int i = 0; i < t->tx_out_count; i++) {
        long int *value = (long int *)malloc(sizeof(long int));
        *value = t->tx_outs[i].value;
        transaction_outpoint outpoint;
        memcpy(outpoint.hash, txid, sizeof(outpoint.hash));
        outpoint.index = i;
        char *outpoint_hash = hash_transaction_outpoint(&outpoint);
        g_hash_table_insert(g_utxo, outpoint_hash, value);
    }

    return true;
}

/**
 * Print every transaction inside the system.
 * @author Ing Tian
 */
void print_all_transactions() {
    printf("--------------------------- TX TABLE --------------------------\n");
    printf("TOTAL TRANSACTION COUNT -------> %u\n", get_total_number_of_transactions());
    g_hash_table_foreach(g_global_transaction_table, print_transaction, NULL);
    printf("\n");
}

/**
 * Print UTXO inside the system.
 * @author Ing Tian
 */
void print_utxo() {
    printf("**************************** UTXO *****************************\n");
    g_hash_table_foreach(g_utxo, print_utxo_entry, NULL);
    printf("\n");
}

/**
 * Get a transaction by its txid.
 * @return A new transaction
 * @author Junjian Chen
 */
transaction *get_transaction_by_txid(char *txid) {
    transaction *t = g_hash_table_lookup(g_global_transaction_table, txid);
    return t;
}

/**
 * Create a new transaction based on two arrays of inputs
 * and outputs.
 * @param transaction_data The incoming arrays of inputs and outputs.
 * @param dest Where the result transaction will be written into.
 * @return True for success, false otherwise.
 * @author Ing Tian
 */
bool create_new_transaction_shortcut(transaction_create_shortcut *transaction_data, transaction *dest) {
    transaction *ret_tx =
        create_an_empty_transaction(transaction_data->num_of_inputs, transaction_data->num_of_outputs);
    for (int i = 0; i < transaction_data->num_of_inputs; i++) {
        transaction_create_shortcut_input curr_input_data = transaction_data->inputs[i];

        if (!g_hash_table_contains(g_global_transaction_table, curr_input_data.previous_txid)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to find the previous transaction with the given TXID: %s",
                        curr_input_data.previous_txid);
            return false;
        }
        transaction *previous_tx = g_hash_table_lookup(g_global_transaction_table, curr_input_data.previous_txid);

        if (curr_input_data.previous_output_idx >= previous_tx->tx_out_count) {
            general_log(LOG_SCOPE, LOG_ERROR, "Previous output index (%u) is out of scope (%u).",
                        curr_input_data.previous_output_idx, previous_tx->tx_out_count);
            return false;
        }
        transaction_output previous_tx_output = previous_tx->tx_outs[curr_input_data.previous_output_idx];

        transaction_input input = {.previous_outpoint = {.index = curr_input_data.previous_output_idx},
                                   .sequence = 1,
                                   .script_bytes = 64,
                                   .signature_script = (char *)malloc(64)};
        memcpy(input.previous_outpoint.hash, transaction_data->inputs[i].previous_txid, 64);
        char *msg = hash_transaction_output(&previous_tx_output);
        secp256k1_ecdsa_signature *signature = sign((unsigned char *)curr_input_data.private_key, (unsigned char *)msg);
        memcpy(input.signature_script, signature->data, 64);
        free(signature);
        free(msg);

        if (!append_new_transaction_input(ret_tx, input, i)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to append input to transaction.");
            return false;
        }
    }

    for (int i = 0; i < transaction_data->num_of_outputs; i++) {
        transaction_create_shortcut_output curr_output_data = transaction_data->outputs[i];

        transaction_output output = {
            .value = curr_output_data.value, .pk_script_bytes = 64, .pk_script = (char *)malloc(64)};
        memcpy(output.pk_script, curr_output_data.public_key, 64);

        if (!append_new_transaction_output(ret_tx, output, i)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to append transaction output.");
            return false;
        }
    }

    *dest = *ret_tx;

    return true;
}
