#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_TRANSACTION_TRANSACTION_PERSISTENCE_H
#include <stdbool.h>

#include "transaction.h"

bool initialize_transaction_persistence();
bool save_transaction(transaction *);
transaction *get_transaction(char *);
bool destroy_transaction_persistence();

#endif
