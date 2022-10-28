#include "block.h"

#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils/constants.h"
#include "utils/cryptography.h"

#define MAXIMUM_TX_PER_BLOCK 100

#define LOG_SCOPE "block"

static GHashTable *g_global_block_table;
char *g_genesis_block_hash;

int get_current_unix_time() {
    time_t tick;
    tick = time(NULL);
    *localtime(&tick);

    return (int)tick;
}

char *sha256_twice(block_header *header) {
    char *hash = hash_struct_in_hex(header, sizeof(header));
    hash = hash_struct_in_hex(hash, sizeof(hash));
    return hash;
}

block *initialize_block_system() {
    g_global_block_table = g_hash_table_new(g_str_hash, g_str_equal);

    block *genesis_block = create_an_empty_block();

    g_genesis_block_hash = sha256_twice(genesis_block->header);

    finalize_block(genesis_block);

    general_log(LOG_SCOPE, LOG_INFO, "Initialized the block library.");

    return genesis_block;
}

void destroy_cryptography_system() {
    //    g_hash_table_foreach(g_global_block_table, free_transaction_table_entry, NULL);
    free(g_global_block_table);

    general_log(LOG_SCOPE, LOG_INFO, "Destroyed the block module.");
}

block *create_an_empty_block() {
    block *block_create = malloc(sizeof(block));
    block_header *header = malloc(sizeof(block_header));
    header->version = 0;
    header->nonce = 0;
    header->nBits = 0;
    header->time = get_current_unix_time();
    block_create->header = header;
    block_create->txn_count = 0;
    block_create->txns = (transaction *)malloc(sizeof(transaction) * MAXIMUM_TX_PER_BLOCK);

    return block_create;
}

void destroy_block(block *block_destroy) {
    for (int i = 0; i < block_destroy->txn_count; i++) {
        destroy_transaction(block_destroy->txns[i]);
    }
    free(block_destroy->header);
    free(block_destroy);
}

bool append_prev_block(block *prev_block, block *cur_block) {
    if (prev_block == NULL || prev_block->header == NULL || cur_block == NULL) {
        return false;
    }
    // sha256(previous block header) twice
    char *prev_block_header_hash = sha256_twice(prev_block->header);

    cur_block->header->prev_block_header_hash = prev_block_header_hash;

    return true;
}

bool check_block_valid(block *block1) {
    if (block1 == NULL) {
        return false;
    }
    block_header *header = block1->header;
    if (header == NULL) {
        return false;
    }

    if (header->time == NULL) {
        block1->header->time = get_current_unix_time();
    }

    return true;
}

bool finalize_block(block *block_finalize) {
    if (!check_block_valid(block_finalize)) {
        return false;
    }

    // sha256(current block header) twice
    char *cur_block_header_hash = sha256_twice(block_finalize->header);

    g_hash_table_insert(g_global_block_table, cur_block_header_hash, block_finalize);

    return true;
}

block *get_block_by_hash(char *hash) {
    block *block1 = g_hash_table_lookup(g_global_block_table, hash);
    return block1;
}

bool append_transaction_into_block(block *block1, transaction *transaction1, unsigned int input_idx) {
    memcpy(&block1->txns[input_idx], &transaction1, sizeof(transaction1));
    return true;
}

bool verify_block_chain(block *cur_block) {
    block *temp = cur_block;

    while (true) {
        // When temp is genesis block
        if (temp->header->prev_block_header_hash == NULL) {
            char *hash = sha256_twice(temp->header);
            if (strcmp(hash, g_genesis_block_hash) == 0) {
                general_log(LOG_SCOPE, LOG_INFO, "The block is valid!");
                return true;
            } else {
                general_log(LOG_SCOPE, LOG_INFO, "The genesis block is invalid!");
                return false;
            }
        } else {
            // When temp isn't genesis block
            block *prev_block = get_block_by_hash(temp->header->prev_block_header_hash);
            //            printf("%s\n",prev_block->header->prev_block_header_hash);
            if (prev_block == NULL) {
                general_log(LOG_SCOPE, LOG_INFO, "The block is invalid: No previous block found!");
                return false;
            }
            char *hash = sha256_twice(prev_block->header);
            if (strcmp(hash, temp->header->prev_block_header_hash) == 0) {
                temp = get_block_by_hash(temp->header->prev_block_header_hash);
            } else {
                general_log(LOG_SCOPE, LOG_INFO, "The block is invalid: Prev hash doesn't match!");
                return false;
            }
        }
    }
}

char *get_genesis_block_hash() { return g_genesis_block_hash; }
