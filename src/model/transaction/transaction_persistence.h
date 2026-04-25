#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#include <stdbool.h>

#include "transaction.h"

bool initialize_transaction_persistence();
bool save_transaction(transaction *);
bool save_utxo_entry(uint8_t *, long int *);
void print_utxo();
bool remove_utxo_entry(uint8_t *);
bool update_transaction_block_id(unsigned long, uint8_t *);
transaction *get_transaction(uint8_t *);
transaction *get_genesis_transaction();
transaction *get_last_inserted_transaction();
bool does_transaction_exist(uint8_t *);
bool does_utxo_entry_exist(uint8_t *);
bool destroy_transaction_persistence(char *);
unsigned int get_total_number_of_transactions();

#endif
