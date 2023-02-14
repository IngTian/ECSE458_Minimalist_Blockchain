#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_BLOCK_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_BLOCK_PERSISTENCE_H
#include <stdbool.h>

#include "block.h"

bool initialize_block_persistence();
bool save_block(block *);
bool does_block_exist(char *);
block *get_block(char *);
unsigned long get_block_id_in_database(block*);
block **get_all_blocks();
void destroy_block(block *);
bool destroy_block_persistence();

#endif
