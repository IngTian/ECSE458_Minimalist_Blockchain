#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_BLOCK_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_BLOCK_PERSISTENCE_H
#include <stdbool.h>
#include "block.h"

bool initialize_block_persistence();
bool save_block(block *);
block *get_block(char *block_id);
bool destroy_block_persistence();

#endif





