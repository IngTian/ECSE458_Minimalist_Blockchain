#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_BLOCK_PERSISTENCE_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_BLOCK_PERSISTENCE_H
#include <stdbool.h>

#include "block.h"

bool initialize_block_persistence();
bool save_block(block *);
bool does_block_exist(char *block_header_hash);
block *get_block(char *block_header_hash);
bool destroy_block_persistence();

#endif
