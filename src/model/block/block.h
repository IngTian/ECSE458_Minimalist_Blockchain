#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_H

#include <stdbool.h>

#include "../transaction/transaction.h"

/*
 * The following field is for defining blocks.
 * For more details, please visit:
 * https://developer.bitcoin.org/reference/block_chain.html
 */

typedef struct BlockHeader {
    int version;                      // The block version number indicates which set of block validation rules to follow.
    char prev_block_header_hash[65];  // A SHA256(SHA256()) hash in internal byte order of the previous block’s header.
    char merkle_root_hash[65];        // A SHA256(SHA256()) hash in internal byte order.
    unsigned int time;                // The block time is a Unix epoch time when the miner started hashing the header (according to the miner).
    unsigned int nBits;               // An encoded version of the target threshold this block’s header hash must be less than or equal to.
    unsigned int nonce;               // An arbitrary number miners change to modify the header hash for the PoW.
} block_header;

typedef struct Block {
    block_header *header;    // The block header in the format described in the block header section.
    transaction **txns;      // Every transaction in this block, one after another, in raw transaction format.
    unsigned int txn_count;  // The total number of transactions in this block, including the coinbase transaction.
} block;

typedef struct BlockHeaderShortcut{
    int version;
    char prev_block_header_hash[65];  // A SHA256(SHA256()) hash in internal byte order of the previous block’s header.
    char merkle_root_hash[65];        // A SHA256(SHA256()) hash in internal byte order.
    unsigned int time;                // The block time is a Unix epoch time when the miner started hashing the header (according to the miner).
    unsigned int nBits;               // An encoded version of the target threshold this block’s header hash must be less than or equal to.
    unsigned int nonce;               // An arbitrary number miners change to modify the header hash for the PoW.
} block_header_shortcut;

typedef struct TransactionsShortcut{
    unsigned int txn_count;
    transaction **txns;
} transactions_shortcut;


typedef struct BlockCreateShortcut{
    transactions_shortcut *transaction_list;
    block_header_shortcut *header;
}block_create_shortcut;

char *hash_block_header(block_header *header);
block *initialize_block_system();
void destroy_block_system();
block *create_an_empty_block(unsigned int);
void destroy_block(block *);
bool append_prev_block(block *prev_block, block *cur_block);
bool finalize_block(block *);
block *get_block_by_hash(char *);
bool append_transaction_into_block(block *, transaction *, unsigned int input_idx);
bool verify_block_chain(block *);
char *get_genesis_block_hash();
void print_block_chain(block *);
bool verify_transaction(transaction *);
bool create_new_block_shortcut(block_create_shortcut *block_data, block *dest);
#endif
