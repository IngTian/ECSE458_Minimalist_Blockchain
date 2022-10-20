#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_H

#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils/cryptography.h"
#include "utils/constants.h"

#define TOTAL_COIN_NUMBER 999999
#define POOL_TXID 114514
#define MAXIMUM_OUTPUT_PER_TX 10
#define MAXIMUM_INPUT_PER_TX 10

/*
 * The following field is for defining transactions.
 * For more details, please visit:
 * https://developer.bitcoin.org/reference/transactions.html#raw-transaction-format
 */

typedef unsigned int TXID;

typedef struct TransactionOutpoint {
    TXID hash;           // The transaction ID (TXID) of the transaction holding the output to spend.
    unsigned int index;  // The output index of the specific output to spend from the transaction. Starts from 0.
} transaction_outpoint;

typedef struct TransactionInput {
    transaction_outpoint *outpoint;  // The previous outpoint being spent.
    unsigned int script_size;        // The number of bytes in the signature script. Maximum is 10,000 bytes.
    char *signature_script;          // A script-language script.
    unsigned int sequence;           // Sequence number.
} transaction_input;


typedef struct TransactionOutput {
    long int value;                // Number of crypto to spend.
    unsigned int pk_script_bytes;  // Number of bytes in the pubkey script.
    char *pk_script;               // Defines the conditions which must be met to spend this output.
} transaction_output;

typedef struct Transaction {
    int version;                   // Transaction version number. Default is 1.
    unsigned int tx_in_count;      // Number of transaction inputs.
    transaction_input **tx_ins;    // Array of transaction inputs.
    unsigned int tx_out_count;     // Number of transaction outputs.
    transaction_output **tx_outs;  // Array of transaction outputs.
    unsigned int lock_time;        // A time number.
} transaction;

void initialize_transaction_system();
void get_all_transaction();
TXID get_transaction_txid(transaction *);
char* convert_txid_to_str(TXID);
public_key get_transaction_output_public_key(transaction_output *);
bool check_transaction_format(transaction*);
int register_transaction_in_system(transaction *, bool);
int register_coin_pool();
transaction* create_new_transaction();
int transaction_receive_coin(transaction *t, transaction_outpoint* outpoint);
int transaction_send_coin(transaction *t,long int value);
int print_UTXO();

#endif
