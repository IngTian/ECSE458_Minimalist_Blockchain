#include "transaction.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "model/block/block.h"
#include "transaction_persistence.h"
#include "utils/constants.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "transaction"

static unsigned int g_total_number_of_transactions;  // The total number of transactions in the system.
static GHashTable *g_utxo;                           // Unspent Transaction Output. mapping each transaction output to its value left.
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
bool verify_transaction_input(transaction_input *i, bool skip_UTXO_check) {
    transaction_outpoint outpoint = i->previous_outpoint;
    char *transaction_hash = outpoint.hash;
    unsigned int output_idx = outpoint.index;

    if (!does_transaction_exist(transaction_hash)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Could not find previous transaction");
        return false;
    }

    transaction *previous_transaction = get_transaction(transaction_hash);

    if (output_idx >= previous_transaction->tx_out_count) {
        general_log(
            LOG_SCOPE, LOG_ERROR, "The output index (%u) is bigger than the output size (%u).", output_idx, previous_transaction->tx_out_count);
        return false;
    }

    transaction_output previous_transaction_output = previous_transaction->tx_outs[output_idx];
    char *hash_msg = hash_transaction_output(&previous_transaction_output);
    secp256k1_pubkey pubkey;
    secp256k1_ecdsa_signature signature;
    memcpy(pubkey.data, previous_transaction_output.pk_script, 64);
    memcpy(signature.data, i->signature_script, 64);

    transaction_outpoint *copied_outpoint = (transaction_outpoint *)malloc(sizeof(transaction_outpoint));
    memcpy(copied_outpoint->hash, outpoint.hash, 64);
    copied_outpoint->hash[64] = '\0';
    copied_outpoint->index = outpoint.index;
    char *utxo_key = hash_transaction_outpoint(copied_outpoint);
    if (!g_hash_table_contains(g_utxo, utxo_key) && !skip_UTXO_check) {
        general_log(LOG_SCOPE, LOG_ERROR, "UTXO is over spent.");
        return false;
    }
    free(utxo_key);

    bool result = verify(&pubkey, (unsigned char *)hash_msg, &signature);
    free(hash_msg);

    if (!result) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to verify signature.");
    }

    return result;
}

/**
 * Free a transaction input.
 * @param input
 * @author Ing Tian
 */
void free_transaction_input(transaction_input *input) {
    if (input != NULL) {
        if (input->signature_script != NULL) free(input->signature_script);
    }
}

/**
 * Free a transaction output.
 * @param output
 * @author Ing Tian
 */
static void free_transaction_output(transaction_output *output) { free(output->pk_script); }

void print_utxo_entry(void *h, void *v, void *user_data) {
    char *hash = (char *)h;
    long int *value = (long int *)v;
    printf("ID: %s VAL: %ld\n", hash, *value);
}

void free_utxo_table_key(void *key) { free(key); }

void free_utxo_table_val(void *val) { free(val); }

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
    g_utxo = g_hash_table_new_full(g_str_hash, g_str_equal, free_utxo_table_key, free_utxo_table_val);
    initialize_transaction_persistence();
    transaction *genesis_transaction = create_an_empty_transaction(1, 1);
    genesis_transaction->tx_ins[0].signature_script = (char *)malloc(65);
    genesis_transaction->tx_ins[0].signature_script[0] = 'A';
    genesis_transaction->tx_ins[0].sequence = 1;
    genesis_transaction->tx_ins[0].script_bytes = 1;
    genesis_transaction->tx_outs[0].value = TOTAL_NUMBER_OF_COINS;
    g_genesis_private_key = (char *)get_a_new_private_key();
    g_genesis_public_key = get_a_new_public_key(g_genesis_private_key);
    genesis_transaction->tx_outs->pk_script = (char *)malloc(65);
    genesis_transaction->tx_outs->pk_script[64] = '\0';
    memcpy(genesis_transaction->tx_outs->pk_script, g_genesis_public_key->data, 64);
    genesis_transaction->tx_outs[0].pk_script_bytes = 64;

    char *genesis_txid = get_transaction_txid(genesis_transaction);

    transaction_outpoint outpoint = {.index = 0};
    memcpy(outpoint.hash, genesis_txid, 64);
    outpoint.hash[64] = '\0';
    char *outpoint_hash = hash_transaction_outpoint(&outpoint);
    long int *genesis_balance = (long int *)malloc(sizeof(long int));
    *genesis_balance = TOTAL_NUMBER_OF_COINS;

    g_hash_table_insert(g_utxo, outpoint_hash, genesis_balance);
    save_transaction(genesis_transaction);

    general_log(LOG_SCOPE, LOG_INFO, "Initialized the transaction module. Genesis TXID: %s", genesis_txid);

    return genesis_transaction;
}

/**
 * Destroy the transaction system.
 * @author Ing Tian
 */
void destroy_transaction_system() {
    g_hash_table_remove_all(g_utxo);
    g_hash_table_destroy(g_utxo);
    destroy_transaction_persistence();
    general_log(LOG_SCOPE, LOG_INFO, "Destroyed the transaction module.");
}

/**
 * Get the transaction_id of a transaction.
 * @param transaction
 * @return The hash of the transaction (32 bytes).
 * @author Ing Tian
 */
char *get_transaction_txid(transaction *t) {
    transaction *copied_tx = (transaction *)malloc(sizeof(transaction));
    memset(copied_tx, 0, sizeof(transaction));
    copied_tx->tx_out_count = t->tx_out_count;
    copied_tx->tx_in_count = t->tx_in_count;
    copied_tx->lock_time = t->lock_time;
    copied_tx->version = t->version;
    char *result = hash_struct_in_hex(copied_tx, sizeof(transaction));
    free(copied_tx);
    return result;
}

/**
 * Get the SHA256 hashcode of a transaction output.
 * @param output A transaction output.
 * @return The SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_transaction_output(transaction_output *output) {
    unsigned long total_size_needed = sizeof(transaction_output) + output->pk_script_bytes;
    transaction_output *copied_output = (transaction_output *)malloc(total_size_needed);
    memset(copied_output, 0, total_size_needed);
    copied_output->pk_script_bytes = output->pk_script_bytes;
    copied_output->value = output->value;
    memcpy(copied_output + sizeof(transaction_output), output->pk_script, output->pk_script_bytes);
    char *result = hash_struct_in_hex(copied_output, total_size_needed);
    free(copied_output);
    return result;
}

/**
 * Get the SHA256 hashcode of a transaction outpoint.
 * @param outpoint A transaction outpoint.
 * @return The SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_transaction_outpoint(transaction_outpoint *outpoint) {
    transaction_outpoint *copied_transaction_outpoint = (transaction_outpoint *)malloc(sizeof(transaction_outpoint));
    memset(copied_transaction_outpoint, 0, sizeof(transaction_outpoint));
    memcpy(copied_transaction_outpoint->hash, outpoint->hash, 65);
    copied_transaction_outpoint->index = outpoint->index;
    char *result = hash_struct_in_hex(copied_transaction_outpoint, sizeof(transaction_outpoint));
    free(copied_transaction_outpoint);
    return result;
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
    memset(t, 0, sizeof(transaction));
    t->version = 1;
    t->tx_in_count = num_of_inputs;
    t->tx_ins = (transaction_input *)malloc(sizeof(transaction_input) * num_of_inputs);
    memset(t->tx_ins, 0, num_of_inputs * sizeof(transaction_input));
    t->tx_out_count = num_of_outputs;
    t->tx_outs = (transaction_output *)malloc(sizeof(transaction_output) * num_of_outputs);
    memset(t->tx_outs, 0, num_of_outputs * sizeof(transaction_output));
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
    free(t->tx_ins);
    for (int i = 0; i < t->tx_out_count; i++) free_transaction_output(&t->tx_outs[i]);
    free(t->tx_outs);
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
    if (!verify_transaction_input(&input, false)) return false;
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
        transaction *previous_transaction = get_transaction(previous_transaction_id);
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
    save_transaction(t);

    // Update UTXO.
    for (int i = 0; i < t->tx_in_count; i++) {
        char *outpoint_hash = hash_transaction_outpoint(&t->tx_ins[i].previous_outpoint);
        g_hash_table_remove(g_utxo, outpoint_hash);
        free(outpoint_hash);
    }

    for (int i = 0; i < t->tx_out_count; i++) {
        long int *value = (long int *)malloc(sizeof(long int));
        *value = t->tx_outs[i].value;
        transaction_outpoint *outpoint = (transaction_outpoint *)malloc(sizeof(transaction_outpoint));
        memcpy(outpoint->hash, txid, 64);
        outpoint->hash[64] = '\0';
        outpoint->index = i;
        char *outpoint_hash = hash_transaction_outpoint(outpoint);
        g_hash_table_insert(g_utxo, outpoint_hash, value);
    }

    return true;
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
 * Print UTXO inside the system.
 * @author Ing Tian
 */
void print_target_utxo(GHashTable *target_utxo) {
    printf("**************************** UTXO *****************************\n");
    g_hash_table_foreach(target_utxo, print_utxo_entry, NULL);
    printf("\n");
}

/**
 * Get a transaction by its txid.
 * @return A new transaction
 * @author Junjian Chen
 */
transaction *get_transaction_by_txid(char *txid) {
    transaction *t = get_transaction(txid);
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
    transaction *ret_tx = create_an_empty_transaction(transaction_data->num_of_inputs, transaction_data->num_of_outputs);
    for (int i = 0; i < transaction_data->num_of_inputs; i++) {
        transaction_create_shortcut_input curr_input_data = transaction_data->inputs[i];

        if (!does_transaction_exist(curr_input_data.previous_txid)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to find the previous transaction with the given TXID: %s", curr_input_data.previous_txid);
            return false;
        }
        transaction *previous_tx = get_transaction(curr_input_data.previous_txid);

        if (curr_input_data.previous_output_idx >= previous_tx->tx_out_count) {
            general_log(LOG_SCOPE,
                        LOG_ERROR,
                        "Previous output index (%u) is out of scope (%u).",
                        curr_input_data.previous_output_idx,
                        previous_tx->tx_out_count);
            return false;
        }
        transaction_output previous_tx_output = previous_tx->tx_outs[curr_input_data.previous_output_idx];

        transaction_input input = {.previous_outpoint = {.index = curr_input_data.previous_output_idx},
                                   .sequence = 1,
                                   .script_bytes = 64,
                                   .signature_script = (char *)malloc(65)};
        input.signature_script[64] = '\0';
        memcpy(input.previous_outpoint.hash, transaction_data->inputs[i].previous_txid, 64);
        input.previous_outpoint.hash[64] = '\0';
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

        transaction_output output = {.value = curr_output_data.value, .pk_script_bytes = 64, .pk_script = (char *)malloc(65)};
        output.pk_script[64] = '\0';
        memcpy(output.pk_script, curr_output_data.public_key, 64);

        append_new_transaction_output(ret_tx, output, i);
    }

    *dest = *ret_tx;

    return true;
}

/**
 * Verify a transaction
 * @param t the transaction being verified
 * @return true if it is valid, false otherwise
 */
bool verify_transaction(transaction *t) {
    unsigned long input_sum = 0, output_sum = 0;

    // Check the validity of each input, and record its unspent amount.
    for (int i = 0; i < t->tx_in_count; i++) {
        if (!verify_transaction_input(&t->tx_ins[i], true)) {
            general_log(LOG_SCOPE, LOG_ERROR, "One of the transaction input is invalid.");
            return false;
        }

        unsigned int previous_output_index = t->tx_ins[i].previous_outpoint.index;
        transaction *previous_transaction = get_transaction(t->tx_ins[i].previous_outpoint.hash);
        input_sum += previous_transaction->tx_outs[previous_output_index].value;
    }

    // Iterate over the outputs and sum up the output sum.
    for (int i = 0; i < t->tx_out_count; i++) {
        output_sum += t->tx_outs[i].value;
    }

    if (input_sum != output_sum) {
        general_log(LOG_SCOPE, LOG_ERROR, "Transaction verify: Input sum (%ld) does not equal to output sum (%ld).", input_sum, output_sum);
        return false;
    }

    return true;
}
