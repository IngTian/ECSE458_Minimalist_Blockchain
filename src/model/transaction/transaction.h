#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_H

#include <stdbool.h>

#include "utils/cryptography.h"

/*
 * The following field is for defining transactions.
 * For more details, please visit:
 * https://developer.bitcoin.org/reference/transactions.html#raw-transaction-format
 */

/*
 * -----------------------------------------------------------
 * Original Structs
 * -----------------------------------------------------------
 */

typedef struct TransactionOutpoint {
    char hash[65];       // The transaction ID (TXID) of the transaction holding the output to spend.
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

typedef struct TransactionCreateShortcutInput {
    char *previous_txid;
    unsigned int previous_output_idx;
    char *private_key;
} transaction_create_shortcut_input;

typedef struct TransactionCreateShortcutOutput {
    long int value;
    char *public_key;
} transaction_create_shortcut_output;

typedef struct TransactionCreateShortcut {
    transaction_create_shortcut_input *inputs;
    unsigned int num_of_inputs;
    transaction_create_shortcut_output *outputs;
    unsigned int num_of_outputs;
} transaction_create_shortcut;

/*
 * -----------------------------------------------------------
 * Socket Structs
 * -----------------------------------------------------------
 */

typedef struct SocketTransactionOutpoint {
    char hash[65];       // The transaction ID (TXID) of the transaction holding the output to spend.
    unsigned int index;  // The output index of the specific output to spend from the transaction. Starts from 0.
} socket_transaction_outpoint;

typedef struct SocketTransactionInput {
    socket_transaction_outpoint previous_outpoint;  // The previous outpoint being spent.
    unsigned int script_bytes;               // The number of bytes in the signature script. Maximum is 10,000 bytes.
    char signature_script[64];                  // A script-language script.
    unsigned int sequence;                   // Sequence number.
} socket_transaction_input;

typedef struct SocketTransactionOutput {
    long int value;                // Number of crypto to spend.
    unsigned int pk_script_bytes;  // Number of bytes in the pubkey script.
    char pk_script[64];               // Defines the conditions which must be met to spend this output.
} socket_transaction_output;

typedef struct SocketTransaction {
    int version;                  // Transaction version number. Default is 1.
    unsigned int tx_in_count;     // Number of transaction inputs.
    unsigned int tx_out_count;    // Number of transaction outputs.
    unsigned int lock_time;       // A time number.
    char transaction_input[0];    // Array of transaction inputs.
    char transaction_output[0];  // Array of transaction outputs.

} socket_transaction;

/*
 * -----------------------------------------------------------
 * Methods
 * -----------------------------------------------------------
 */

transaction *initialize_transaction_system();
void destroy_transaction_system();
char *get_transaction_txid(transaction *);
char *get_genesis_transaction_private_key();
secp256k1_pubkey *get_genesis_transaction_public_key();
transaction *get_transaction_by_txid(char *);
void destroy_transaction(transaction *);
bool create_new_transaction_shortcut(transaction_create_shortcut *, transaction *);
bool finalize_transaction(transaction *);
void print_utxo();
socket_transaction *cast_to_socket_transaction(transaction*);
transaction *cast_to_transaction(socket_transaction *);
#endif
