#include "block.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "model/block/block_persistence.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "block"

char *g_genesis_block_hash;  // The hash of the header of the genesis block.

/*
 * -----------------------------------------------------------
 * Helper Methods
 * -----------------------------------------------------------
 */

/**
 * Hash the block header twice with SHA256.
 * @param header A header.
 * @return The SHA256 code.
 * @auhor Junjian Chen
 */
char *hash_block_header(block_header *header) {
    char *hash = hash_struct_in_hex(header, sizeof(block_header));
    char *ret_val = hash_struct_in_hex(hash, sizeof(hash));
    free(hash);
    return ret_val;
}

/*
 * -----------------------------------------------------------
 * APIs
 * -----------------------------------------------------------
 */

/**
 * Initialize the block system.
 * @return The genesis block
 * @author Junjian Chen
 */
block *initialize_block_system() {
    initialize_block_persistence();

    unsigned int total_number_of_blocks = get_total_number_of_blocks();

    if (total_number_of_blocks == 0) {
        block *genesis_block = create_an_empty_block(1);
        g_genesis_block_hash = hash_block_header(genesis_block->header);
        general_log(LOG_SCOPE, LOG_INFO, "Initialized the block system Genesis block hash: %s.", g_genesis_block_hash);
        return genesis_block;
    } else {
        return get_genesis_block();
    }
}

/**
 * Destroy the block system.
 * @author Junjian Chen
 */
void destroy_block_system() {
    destroy_block_persistence();
    free(g_genesis_block_hash);
    general_log(LOG_SCOPE, LOG_INFO, "Destroyed the block module.");
}

/**
 * Create an empty block.
 * @param transaction_amount The amount of transactions in the block.
 * @return An empty block.
 * @author Junjian Chen
 */
block *create_an_empty_block(unsigned int transaction_amount) {
    block *block_create = malloc(sizeof(block));
    block_header *header = malloc(sizeof(block_header));
    header->version = 0;
    memset(header->prev_block_header_hash, '\0', 65);
    memset(header->merkle_root_hash, '\0', 65);
    header->nonce = 0;
    header->nBits = 0;
    header->time = get_current_unix_time();
    block_create->header = header;
    block_create->txn_count = transaction_amount;
    block_create->txns = (transaction **)malloc(sizeof(transaction *) * transaction_amount);
    return block_create;
}

/**
 * Link together the current block and a previous block.
 * @param prev_block The previous block.
 * @param cur_block The current block.
 * @return True for success, and false otherwise.
 * @author Junjian Chen
 */
bool append_prev_block(block *prev_block, block *cur_block) {
    if (prev_block == NULL || prev_block->header == NULL || cur_block == NULL) {
        general_log(LOG_SCOPE, LOG_ERROR, "Blocks cannot be NULL when linking!");
        return false;
    }

    // SHA256(previous block header) twice.
    char *prev_block_header_hash = hash_block_header(prev_block->header);

    memcpy(cur_block->header->prev_block_header_hash, prev_block_header_hash, 64);

    return true;
}

/**
 * Check if a block is valid or not.
 * @param block1 The block to check.
 * @return True for success, and false otherwise.
 * @author Junjian Chen
 */
bool check_block_valid(block *block1) {
    // check if the block is NULL
    if (block1 == NULL) {
        general_log(LOG_SCOPE, LOG_ERROR, "The block is null.");
        return false;
    }

    // check if the header is NULL
    block_header *header = block1->header;
    if (header == NULL) {
        general_log(LOG_SCOPE, LOG_ERROR, "The block header is null.");
        return false;
    }

    // check if the previous block is NULL
    if (strcmp(header->prev_block_header_hash, "") == 0) {
        if (strcmp(hash_block_header(header), g_genesis_block_hash) != 0) {
            general_log(LOG_SCOPE, LOG_ERROR, "The block is invalid since the previous block is null.");
            return false;
        }
    } else {
        block *prev_block = get_block_by_hash(header->prev_block_header_hash);
        if (prev_block == NULL) {
            general_log(LOG_SCOPE, LOG_ERROR, "The block is invalid since the previous block is null.");
            return false;
        }
    }

    // check if the time is valid
    if (header->time == 0) {
        return false;
    }

    return true;
}

/**
 * Register a block in the system.
 * @param block_finalize The block to register.
 * @return True for success, and false otherwise.
 * @author Junjian Chen
 */
bool finalize_block(block *block_finalize) {
    // check if the block is valid
    if (!check_block_valid(block_finalize)) {
        return false;
    }

    return save_block(block_finalize);
}

/**
 * Get a block by its header hash.
 * @param hash The header hash of a block.
 * @return The block.
 * @author Junjian Chen
 */
block *get_block_by_hash(char *hash) { return get_block(hash); }

/**
 * Add transaction into the block.
 * @param block1 The block to contain the transaction.
 * @param transaction1 The transaction to append.
 * @param input_idx The transaction idx.
 * @return True for success, and false otherwise.
 * @author Junjian Chen
 */
bool append_transaction_into_block(block *block1, transaction *transaction1, unsigned int input_idx) {
    block1->txns[input_idx] = transaction1;
    return true;
}

/**
 * Verify transactions in a block
 * @param block1  The block to verify transaction in it
 * @return True for the valid transactions in the block, false for invalid
 * @author Junjian Chen
 */
bool verify_block_transaction(block *block1) {
    for (int i = 0; i < block1->txn_count; i++) {
        if (!verify_transaction(block1->txns[i])) {
            return false;
        }
    }

    return true;
}

/**
 * Verify the integrity of the block chain.
 * @param chain_tail The tail block.
 * @return True for valid, and false otherwise.
 * @author Junjian Chen
 */
bool verify_block_chain(block *chain_tail) {
    block *temp = chain_tail;

    int i = 0;

    while (true) {
        if (!verify_block_transaction(temp)) {
            general_log(
                LOG_SCOPE, LOG_ERROR, "The chain is invalid because one transaction in the block is invalid.\n Error block: the last %dth block", i);
            return false;
        }

        if (strcmp(temp->header->prev_block_header_hash, "") == 0) {
            // When temp is genesis block
            char *hash = hash_block_header(temp->header);
            if (strcmp(hash, g_genesis_block_hash) == 0) {
                general_log(LOG_SCOPE, LOG_INFO, "The chain is valid!");
                return true;
            } else {
                general_log(LOG_SCOPE,
                            LOG_ERROR,
                            "The chain is invalid because the first block does not equal the genesis block.\n Error block: the last %dth block",
                            i);
                return false;
            }
        } else {
            // When temp isn't genesis block
            block *prev_block = get_block_by_hash(temp->header->prev_block_header_hash);
            if (prev_block == NULL) {
                general_log(LOG_SCOPE, LOG_ERROR, "The chain is invalid: no previous block found for a block!\n Error block: the last %dth block", i);
                return false;
            }

            char *hash = hash_block_header(prev_block->header);
            if (strcmp(hash, temp->header->prev_block_header_hash) == 0) {
                temp = get_block_by_hash(temp->header->prev_block_header_hash);
            } else {
                general_log(LOG_SCOPE, LOG_ERROR, "The block is invalid: previous block hash doesn't match!\n Error block: the last %dth block", i);
                return false;
            }
        }

        i++;
    }
}

/**
 * Verify a single block:
 * 1. Verify whether all transactions in this block are valid
 * 2. Verify whether the previous block header is valid
 * @param block1 The block to be verified
 * @return True if valid. False otherwise
 * @author Junjian Chen
 */
bool verify_block(block *block1) {
    if (!verify_block_transaction(block1)) {
        return false;
    }

    if (!check_block_valid(block1)) {
        return false;
    }

    return true;
}

/**
 * Get the hash code of the genesis block.
 * @return The hash of the genesis block.
 * @author Junjian Chen
 */
char *get_genesis_block_hash() { return g_genesis_block_hash; }

/**
 * Create a new block based on its header information, transactions information
 * @param block_data The incomming header, transactions
 * @param dest Where the result block will be written to
 * @return True for success, false otherwise.
 * @author Junjian Chen
 */
bool create_new_block_shortcut(block_create_shortcut *block_data, block *dest) {
    block *ret_block = create_an_empty_block(block_data->transaction_list->txn_count);
    memcpy(ret_block->header->prev_block_header_hash, block_data->header->prev_block_header_hash, 65);
    ret_block->header->time = block_data->header->time;
    memcpy(ret_block->header->merkle_root_hash, block_data->header->merkle_root_hash, 65);
    ret_block->header->nBits = block_data->header->nBits;
    ret_block->header->nonce = block_data->header->nonce;
    ret_block->header->version = block_data->header->version;
    ret_block->txn_count = block_data->transaction_list->txn_count;
    ret_block->txns = block_data->transaction_list->txns;
    *dest = *ret_block;
    return true;
}

bool block_rollback(char *rollback_block_hash, char *current_block_hash) {
    if (rollback_block_hash == NULL) {
        general_log(LOG_SCOPE, LOG_ERROR, "The block hash is null.");
        return false;
    }
    block *rollback_block = get_block_by_hash(rollback_block_hash);
    if (rollback_block == NULL) {
        general_log(LOG_SCOPE, LOG_ERROR, "The block is null.");
        return false;
    }
    block *current_block = get_block_by_hash(current_block_hash);

    while (strcmp(current_block_hash, rollback_block_hash) != 0) {
        current_block_hash = current_block->header->prev_block_header_hash;
        destroy_block(current_block);
        current_block = get_block_by_hash(current_block_hash);
    }
    return true;
}

/**
 * Cast a block to a socket block
 * @param b A block
 * @return A socket block for transmitting
 * @author Junjian Chen
 */
socket_block *cast_to_socket_block(block *b) {
    int txns_total_length = 0;
    int txns_ptr_deviation[b->txn_count + 1];

    for (int i = 0; i < b->txn_count; i++) {
        txns_ptr_deviation[i] = txns_total_length;
        socket_transaction *tmp = cast_to_socket_transaction(b->txns[i]);
        txns_total_length += get_socket_transaction_length(tmp);
        free(tmp);
    }
    txns_ptr_deviation[b->txn_count] = txns_total_length;

    socket_block *socket_blk = (socket_block *)malloc(sizeof(socket_block) + txns_total_length);
    socket_blk->version = b->header->version;
    socket_blk->nonce = b->header->version;
    socket_blk->txn_count = b->txn_count;
    socket_blk->nBits = b->header->nBits;
    socket_blk->time = b->header->time;
    memcpy(socket_blk->prev_block_header_hash, b->header->prev_block_header_hash, 65);
    memcpy(socket_blk->merkle_root_hash, b->header->merkle_root_hash, 65);

    for (int i = 0; i < b->txn_count; i++) {
        char *tx_starting_address = socket_blk->txns + txns_ptr_deviation[i];
        socket_transaction *current_socket_tx = (socket_transaction *)(tx_starting_address);
        socket_transaction *tmp = cast_to_socket_transaction(b->txns[i]);
        memcpy(current_socket_tx, tmp, txns_ptr_deviation[i + 1] - txns_ptr_deviation[i]);
        free(tmp);
    }

    socket_blk->txns_size = txns_total_length;

    return socket_blk;
}

/**
 * Cast to a block from a socket block.
 * @param socket_blk A socket block.
 * @return A block.
 * @author Junjian Chen
 */
block *cast_to_block(socket_block *socket_blk) {
    // Initialize a block header.
    block_header *blk_header = (block_header *)malloc(sizeof(block_header));
    blk_header->time = socket_blk->time;
    blk_header->version = socket_blk->version;
    blk_header->nBits = socket_blk->nBits;
    blk_header->nonce = socket_blk->nonce;
    memcpy(blk_header->prev_block_header_hash, socket_blk->prev_block_header_hash, 65);
    memcpy(blk_header->merkle_root_hash, socket_blk->merkle_root_hash, 65);

    // Initialize a block.
    block *blk = (block *)malloc(sizeof(block));
    blk->txn_count = socket_blk->txn_count;
    blk->header = blk_header;
    blk->txns = (transaction **)malloc(blk->txn_count * sizeof(transaction *));

    // Get the total length of txns.
    int total_length = 0;
    for (int i = 0; i < socket_blk->txn_count; i++) {
        // Get current socket transaction.
        socket_transaction *current_socket_tx = (socket_transaction *)(socket_blk->txns + total_length);
        int current_socket_transaction_length = get_socket_transaction_length(current_socket_tx);
        total_length += current_socket_transaction_length;

        // Initialize current tx.
        blk->txns[i] = cast_to_transaction(current_socket_tx);
    }

    return blk;
}

/**
 * Get the socket block length.
 * @param b A block.
 * @return The length of it if it was a socket block.
 * @author Junjian Chen
 */
int get_socket_block_length(block *b) {
    int txns_total_length = sizeof(socket_block);
    for (int i = 0; i < b->txn_count; i++) {
        socket_transaction *tmp = cast_to_socket_transaction(b->txns[i]);
        txns_total_length += get_socket_transaction_length(tmp);
        free(tmp);
    }
    return txns_total_length;
}
