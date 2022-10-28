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

    int version;

    char* prev_block_header_hash;

    int time;

    int nBits;

    int nonce;
} block_header;

typedef struct Block{

    block_header* header;

    transaction **txns;

    unsigned int txn_count;

} block;


block *initialize_block_system();
block *create_an_empty_block();
void destroy_block(block *);
bool append_prev_block(block* prev_block, block* cur_block);
bool finalize_block(block *);
block *get_block_by_hash(char *);
bool append_transaction_into_block(transaction *, block *);
bool verify_block_chain(block *);
char* get_genesis_block_hash();
char* sha256_twice(block_header *);

#endif


