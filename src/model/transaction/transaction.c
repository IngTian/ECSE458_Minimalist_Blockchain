#include "transaction.h"

#include "utils/sys_utils.h"

GHashTable *global_transaction_table;
GHashTable *utxo;

/**
 * Setup the transaction system, including setting up:
 * 1. The global transaction table.
 * 2. Unspent Transaction Output (UTXO)
 */
void initialize_transaction_system() {
    global_transaction_table = g_hash_table_new(g_str_hash, g_str_equal);
    utxo = g_hash_table_new(g_str_hash, g_str_equal);
}

/**
 * Get the hash code of a transaction.
 * @param transaction
 * @return TXID
 */
TXID get_transaction_txid(transaction *transaction) { return g_direct_hash(transaction); }

/**
 * Convert TXID to string.
 * @param id TXID
 * @return TXID in string.
 */
char *convert_txid_to_str(TXID id) {
    char *res = (char *)malloc(8);
    sprintf(res, "%x", id);
    return res;
}

/**
 * Retrieve the target public key from a transaction output.
 * @param output
 * @return The public key buried in the output.
 */
public_key get_transaction_output_public_key(transaction_output *output) { return output->pk_script; }

/**
 * Check if the transaction is valid. Essentially, the sum of
 * outputs must be smaller than that of the input.
 * @param transaction A transaction.
 * @return true for valid, and false otherwise.
 */
bool check_transaction_valid(transaction* transaction) {

}

/**
 * Add a transaction to the network.
 * @param transaction
 * @return 0 for success, other numbers indicate failure.
 */
int register_transaction_in_system(transaction *transaction) {
    TXID transaction_id = get_transaction_txid(transaction);
    char* txid_str = convert_txid_to_str(transaction_id);
    g_hash_table_insert(global_transaction_table, txid_str, transaction);
    return 0;
}
