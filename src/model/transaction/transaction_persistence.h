#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#include <stdbool.h>

#include "transaction.h"

bool initialize_transaction_persistence();
bool save_transaction(transaction *);
bool update_transaction_block_id(unsigned long, char *);
transaction *get_transaction(char *);
bool does_transaction_exist(char *);
bool destroy_transaction_persistence();

#endif
