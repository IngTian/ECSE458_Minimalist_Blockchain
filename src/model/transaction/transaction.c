#include "transaction.h"

#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"

#define MAXIMUM_OUTPUT_PER_TX 10
#define MAXIMUM_INPUT_PER_TX 10
#define TOTAL_NUMBER_OF_COINS 4096

static unsigned int g_total_number_of_transactions;  // The total number of transactions in the system.
static GHashTable *g_global_transaction_table;       // The global transaction table, mapping TXID to transaction.
static GHashTable *g_utxo;  // Unspent Transaction Output. mapping each transaction output to its value left.
char *g_genesis_private_key;
secp256k1_pubkey *g_genesis_public_key;

/*
 * -----------------------------------------------------------
 * Helper Methods
 * -----------------------------------------------------------
 */
/**
 * Return the SHA256 hashcode of a general
 * struct.
 * @param ptr A pointer.
 * @param size The size, in bytes, to hash.
 * @return SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_struct(void *ptr, unsigned int size) {
    char *msg = (char *)malloc(size + 1);
    memcpy(msg, ptr, size);
    msg[size] = '\0';
    char *hash_msg = hash_sha256(msg);
    free(msg);
    return hash_msg;
}

/**
 * Get the SHA256 hashcode of a transaction output.
 * @param output A transaction output.
 * @return The SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_transaction_output(transaction_output *output) { return hash_struct(output, sizeof(transaction_output)); }

/**
 * Get the SHA256 hashcode of a transaction outpoint.
 * @param outpoint A transaction outpoint.
 * @return The SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_transaction_outpoint(transaction_outpoint *outpoint) {
    return hash_struct(outpoint, sizeof(transaction_outpoint));
}

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
    char *public_key = previous_transaction_output.pk_script;
    char *hash_msg = hash_transaction_output(&previous_transaction_output);
    secp256k1_pubkey pubkey;
    secp256k1_ecdsa_signature signature;
    memcpy(pubkey.data, previous_transaction_output.pk_script, 64);
    memcpy(signature.data, i->signature_script, 64);

    bool result = verify(&pubkey, (unsigned char *)hash_msg, &signature);
    free(hash_msg);
    return result;
}

/**
 * Free a transaction input.
 * @param input
 * @author Ing Tian
 */
void free_transaction_input(transaction_input *input) { free(input->signature_script); }

/**
 * Free a transaction output.
 * @param output
 * @author Ing Tian
 */
static void free_transaction_output(transaction_output *output) { free(output->pk_script); }

void print_transaction(char *txid, transaction *t, void *user_data) {
    printf("--------------------------- TX TABLE --------------------------\n");
    printf("TXID: %s VERSION: %d TX IN COUNT: %u TX OUT COUNT: %u LOCK: %u\n", convert_char_hexadecimal(txid, 32),
           t->version, t->tx_in_count, t->tx_out_count, t->lock_time);
    printf("<Printing Inputs>\n");
    for (int i = 0; i < t->tx_in_count; i++) {
        transaction_input input = t->tx_ins[i];
        printf("SEQ: %u SIG: %s OUTPOINT HASH: %s OUTPOINT ID: %u\n", input.sequence,
               convert_char_hexadecimal(input.signature_script, 64),
               convert_char_hexadecimal(input.previous_outpoint.hash, 32), input.previous_outpoint.index);
    }
    printf("<Printing Outputs>\n");
    for (int i = 0; i < t->tx_out_count; i++) {
        transaction_output output = t->tx_outs[i];
        printf("VAL: %ld PK: %s\n", output.value, convert_char_hexadecimal(output.pk_script, 64));
    }
}

void print_utxo_entry(char *hash, long int *val, void *user_data) {
    printf("**************************** UTXO *****************************\n");
    printf("ID: %s VAL: %ld\n", convert_char_hexadecimal(hash, 32), *val);
}

/*
 * -----------------------------------------------------------
 * APIs
 * -----------------------------------------------------------
 */
/**
 * Setup the transaction system, including setting up:
 * 1. The global transaction table.
 * 2. Unspent Transaction Output (UTXO)
 * @author Ing Tian
 */
void initialize_transaction_system() {
    g_global_transaction_table = g_hash_table_new(g_str_hash, g_str_equal);
    g_utxo = g_hash_table_new(g_str_hash, g_str_equal);

    transaction *genesis_transaction = create_an_empty_transaction();
    genesis_transaction->tx_in_count = 1;
    genesis_transaction->tx_out_count = 1;
    genesis_transaction->tx_ins[0].signature_script = "GENESIS";
    genesis_transaction->tx_ins[0].sequence = 1;
    genesis_transaction->tx_ins[0].script_bytes = 7;
    genesis_transaction->tx_outs[0].value = TOTAL_NUMBER_OF_COINS;
    g_genesis_private_key = (char *)get_a_new_private_key();
    g_genesis_public_key = get_a_new_public_key(g_genesis_private_key);
    genesis_transaction->tx_outs->pk_script = (char *)malloc(64);
    memcpy(genesis_transaction->tx_outs->pk_script, g_genesis_public_key->data, 64);
    genesis_transaction->tx_outs[0].pk_script_bytes = 64;

    char *genesis_txid = get_transaction_txid(genesis_transaction);
    transaction_outpoint outpoint;
    memcpy(outpoint.hash, genesis_txid, 32);
    outpoint.index = 0;
    char *outpoint_hash = hash_transaction_outpoint(&outpoint);
    long int *genesis_balance = (long int *)malloc(sizeof(long int));
    *genesis_balance = TOTAL_NUMBER_OF_COINS;
    g_hash_table_insert(g_utxo, outpoint_hash, genesis_balance);
    g_hash_table_insert(g_global_transaction_table, genesis_txid, genesis_transaction);
}

/**
 * Get the transaction_id of a transaction.
 * @param transaction
 * @return The hash of the transaction (32 bytes).
 * @author Ing Tian
 */
char *get_transaction_txid(transaction *t) {
    unsigned int transaction_block_size = sizeof(transaction);
    char *msg = (char *)malloc(transaction_block_size + 1);
    memcpy(msg, t, transaction_block_size);
    msg[transaction_block_size] = '\0';
    char *hash = hash_sha256(msg);
    free(msg);
    return hash;
}

/**
 * Get the total number of transactions in the system.
 * @return The total number of transactions in the system.
 * @author Ing Tian
 */
unsigned int get_total_number_of_transactions() { return g_total_number_of_transactions; }

/**
 * Create an empty transaction.
 * @return An empty transaction.
 * @author Ing Tian
 */
transaction *create_an_empty_transaction() {
    transaction *t = (transaction *)malloc(sizeof(transaction));
    t->version = 1;
    t->tx_in_count = 0;
    t->tx_ins = (transaction_input *)malloc(sizeof(transaction_input) * MAXIMUM_INPUT_PER_TX);
    t->tx_out_count = 0;
    t->tx_outs = (transaction_output *)malloc(sizeof(transaction_output) * MAXIMUM_OUTPUT_PER_TX);
    t->lock_time = get_timestamp();
    return t;
};

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
bool append_new_transaction_input(transaction *t, transaction_input input) {
    if (t->tx_in_count + 1 >= MAXIMUM_INPUT_PER_TX || !verify_transaction_input(&input)) return false;

    t->tx_ins[t->tx_in_count++] = input;
    return true;
}

/**
 * Append a new transaction output to a transaction.
 * @param t A transaction.
 * @param output A new output.
 * @return True for success, false otherwise.
 * @auhor Ing Tian
 */
bool append_new_transaction_output(transaction *t, transaction_output output) {
    if (t->tx_out_count + 1 >= MAXIMUM_OUTPUT_PER_TX) return false;

    t->tx_outs[t->tx_out_count++] = output;
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

    if (input_sum != output_sum) return false;

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
void print_all_transactions() { g_hash_table_foreach(g_global_transaction_table, print_transaction, NULL); }

/**
 * Print UTXO inside the system.
 * @author Ing Tian
 */
void print_utxo() { g_hash_table_foreach(g_utxo, print_utxo_entry, NULL); }
