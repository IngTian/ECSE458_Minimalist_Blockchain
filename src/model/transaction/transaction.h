#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_H

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/constants.h"
#include "utils/cryptography.h"

/*
 * The following field is for defining transactions.
 * For more details, please visit:
 * https://developer.bitcoin.org/reference/transactions.html#raw-transaction-format
 */

typedef struct TransactionOutpoint {
    char hash[64];       // The transaction ID (TXID) of the transaction holding the output to spend.
    unsigned int index;  // The output index of the specific output to spend from the transaction. Starts from 0.
} transaction_outpoint;

typedef struct TransactionInput {
    transaction_outpoint previous_outpoint;  // The previous outpoint being spent.
    unsigned int script_bytes;               // The number of bytes in the signature script. Maximum is 10,000 bytes.
    char *signature_script;                  // A script-language script.
    unsigned int sequence;                   // Sequence number.
} transaction_input;

typedef struct TransactionOutput {
    long int value;                // Number of crypto to spend.
    unsigned int pk_script_bytes;  // Number of bytes in the pubkey script.
    char *pk_script;               // Defines the conditions which must be met to spend this output.
} transaction_output;

typedef struct Transaction {
    int version;                  // Transaction version number. Default is 1.
    unsigned int tx_in_count;     // Number of transaction inputs.
    transaction_input *tx_ins;    // Array of transaction inputs.
    unsigned int tx_out_count;    // Number of transaction outputs.
    transaction_output *tx_outs;  // Array of transaction outputs.
    unsigned int lock_time;       // A time number.
} transaction;

transaction *initialize_transaction_system();
char *get_transaction_txid(transaction *);
char *hash_transaction_outpoint(transaction_outpoint*);
char *hash_transaction_output(transaction_output*);
unsigned int get_total_number_of_transactions();
char *get_genesis_transaction_private_key();
secp256k1_pubkey *get_genesis_transaction_public_key();
transaction *create_an_empty_transaction();
void destroy_transaction(transaction *);
bool append_new_transaction_input(transaction *, transaction_input);
bool append_new_transaction_output(transaction *, transaction_output);
bool finalize_transaction(transaction *);
void print_all_transactions();
void print_utxo();
#endif
