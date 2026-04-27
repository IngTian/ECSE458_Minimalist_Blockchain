#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_MODEL_BLOCK_H

#include <stdbool.h>
#include <stdint.h>

#include "../transaction/transaction.h"

/*
 * The following field is for defining blocks.
 * For more details, please visit:
 * https://developer.bitcoin.org/reference/block_chain.html
 */

typedef struct BlockHeader {
    int version;                        // The block version number indicates which set of block validation rules to follow.
    uint8_t prev_block_header_hash[32]; // SHA256(SHA256()) of the previous block header (raw bytes).
    uint8_t merkle_root_hash[32];       // SHA256(SHA256()) merkle root (raw bytes).
    unsigned int time;                  // Unix epoch time when the miner started hashing the header.
    unsigned int nBits;                 // Encoded target threshold.
    unsigned int nonce;                 // PoW nonce.
} block_header;

typedef struct Block {
    block_header *header;    // The block header in the format described in the block header section.
    transaction **txns;      // Every transaction in this block, one after another, in raw transaction format.
    unsigned int txn_count;  // The total number of transactions in this block, including the coinbase transaction.
} block;

typedef struct BlockHeaderShortcut {
    int version;
    uint8_t prev_block_header_hash[32];
    uint8_t merkle_root_hash[32];
    unsigned int time;
    unsigned int nBits;
    unsigned int nonce;
} block_header_shortcut;

typedef struct TransactionsShortcut {
    unsigned int txn_count;
    transaction **txns;
} transactions_shortcut;

typedef struct BlockCreateShortcut {
    transactions_shortcut *transaction_list;
    block_header_shortcut *header;
} block_create_shortcut;

/*
 * -----------------------------------------------------------
 * Socket Structs
 * -----------------------------------------------------------
 */

typedef struct SocketBlock {
    int version;
    uint8_t prev_block_header_hash[32];
    uint8_t merkle_root_hash[32];
    unsigned int time;
    unsigned int nBits;
    unsigned int nonce;
    unsigned int txn_count;
    unsigned int txns_size;
    char txns[0];
} socket_block;

uint8_t *hash_block_header(block_header *header);
block *initialize_block_system(bool skip_genesis);
void destroy_block_system(char *);
block *create_an_empty_block(unsigned int);
bool append_prev_block(block *prev_block, block *cur_block);
bool finalize_block(block *);
block *get_block_by_hash(uint8_t *);
bool append_transaction_into_block(block *, transaction *, unsigned int input_idx);
bool verify_block_chain(block *);
bool verify_block(block *);
uint8_t *get_genesis_block_hash(void);
bool create_new_block_shortcut(block_create_shortcut *block_data, block *dest);
socket_block *cast_to_socket_block(block *);
block *cast_to_block(socket_block *);
int get_socket_block_length(block *);
block *create_a_new_block(uint8_t *previous_block_header_hash, transaction *txn, uint8_t **result_header_hash);
#endif
