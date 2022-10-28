#include "block.h"

#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "block"

static GHashTable *g_global_block_table;  // The global block table that maps block header hash to the block.
char *g_genesis_block_hash;               // The hash of the header of the genesis block.

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

void free_g_global_block_table_entry(void *block_id, void *blk, void *user_data) {
    free(block_id);
    destroy_block(blk);
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
    g_global_block_table = g_hash_table_new(g_str_hash, g_str_equal);
    block *genesis_block = create_an_empty_block(1);
    g_genesis_block_hash = hash_block_header(genesis_block->header);
    finalize_block(genesis_block);
    general_log(LOG_SCOPE, LOG_INFO, "Initialized the block system.");
    return genesis_block;
}

/**
 * Destroy the block system.
 * @author Junjian Chen
 */
void destroy_block_system() {
    g_hash_table_foreach(g_global_block_table, free_g_global_block_table_entry, NULL);
    free(g_global_block_table);
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
    block_create->txn_count = 0;
    block_create->txns = (transaction **)malloc(sizeof(transaction *) * transaction_amount);
    return block_create;
}

/**
 * Free the memory space of a block.
 * @param block_destroy The block to be destroyed.
 * @author Junjian Chen
 */
void destroy_block(block *block_destroy) {
    free(block_destroy->header);
    free(block_destroy);
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
    if (block1 == NULL) {
        return false;
    }

    block_header *header = block1->header;
    if (header == NULL) {
        return false;
    }

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
    if (!check_block_valid(block_finalize)) {
        return false;
    }

    // sha256(current block header) twice
    char *cur_block_header_hash = hash_block_header(block_finalize->header);
    g_hash_table_insert(g_global_block_table, cur_block_header_hash, block_finalize);

    return true;
}

/**
 * Get a block by its header hash.
 * @param hash The header hash of a block.
 * @return The block.
 * @author Junjian Chen
 */
block *get_block_by_hash(char *hash) { return g_hash_table_lookup(g_global_block_table, hash); }

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
 * Verify the integrity of the block chain.
 * @param chain_tail The header block.
 * @return True for valid, and false otherwise.
 * @author Junjian Chen
 */
bool verify_block_chain(block *chain_tail) {
    block *temp = chain_tail;

    while (true) {
        if (strcmp(temp->header->prev_block_header_hash, "") == 0) {
            // When temp is genesis block
            char *hash = hash_block_header(temp->header);
            if (strcmp(hash, g_genesis_block_hash) == 0) {
                general_log(LOG_SCOPE, LOG_INFO, "The chain is valid!");
                return true;
            } else {
                general_log(LOG_SCOPE, LOG_ERROR,
                            "The chain is invalid because the first block does not equal the genesis block.");
                return false;
            }
        } else {
            // When temp isn't genesis block
            block *prev_block = get_block_by_hash(temp->header->prev_block_header_hash);
            if (prev_block == NULL) {
                general_log(LOG_SCOPE, LOG_ERROR, "The chain is invalid: no previous block found for a block!");
                return false;
            }

            char *hash = hash_block_header(prev_block->header);
            if (strcmp(hash, temp->header->prev_block_header_hash) == 0) {
                temp = get_block_by_hash(temp->header->prev_block_header_hash);
            } else {
                general_log(LOG_SCOPE, LOG_INFO, "The block is invalid: previous block hash doesn't match!");
                return false;
            }
        }
    }
}

/**
 * Get the hash code of the genesis block.
 * @return The hash of the genesis block.
 * @author Junjian Chen
 */
char *get_genesis_block_hash() { return g_genesis_block_hash; }
