#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#include <stdbool.h>

#include "transaction.h"

bool initialize_transaction_persistence();
bool save_transaction(transaction *);
bool save_utxo_entry(char *, long int *);
void print_utxo();
bool remove_utxo_entry(char *);
bool update_transaction_block_id(unsigned long, char *);
transaction *get_transaction(char *);
transaction *get_genesis_transaction();
bool does_transaction_exist(char *);
bool does_utxo_entry_exist(char *);
bool destroy_transaction_persistence();
unsigned int get_total_number_of_transactions();

#endif
