#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_H

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/constants.h"
#include "utils/cryptography.h"
#include "../transaction/transaction.h"

typedef struct Block_Header{
    char* hash;
} block_header;

typedef struct Block{

    block_header* current_block_header;

    block_header* prev_block_header;

    transaction **transaction_list;

    unsigned int transaction_count;

    long long timestamp_create;

} block;


block *initialize_block_system();
block *create_an_empty_block();
void destroy_block(block *);
bool append_prev_block(block* prev_block, block* cur_block);
bool finalize_block(block *);
block *get_block_by_hash(char *);
bool append_transaction_into_block(transaction *, block *);




#endif





