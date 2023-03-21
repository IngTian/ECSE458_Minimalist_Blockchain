#include "block_persistence.h"

#include <glib.h>
#include <string.h>

#include "model/transaction/transaction_persistence.h"
#include "utils/constants.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "block_persistence"

static GHashTable *g_global_block_table;  // The global block table that maps block header hash to the block.
block *g_genesis_block = NULL;            // The genesis block.

/**
 * Free the memory space of a block.
 * @param block_destroy The block to be destroyed.
 * @author Junjian Chen
 */
void destroy_block(block *block_destroy) {
    if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        free(block_destroy->header);
        free(block_destroy);
    } else if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        // TODO: Delete an entry of block here.
    }
}

/*
 * -----------------------------------------------------------
 * Helper Methods
 * -----------------------------------------------------------
 */

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
 * Initialize the persistence layer by creating tables
 * in the database.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool initialize_block_persistence() {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query =
            "CREATE TABLE if not exists block\n"
            "(\n"
            "    block_id  int auto_increment,\n"
            "    txn_count int unsigned not null,\n"
            "    primary key (block_id)\n"
            ") ENGINE = %s;\n"
            "\n"
            "CREATE TABLE if not exists block_header\n"
            "(\n"
            "    block_h_id             int          not null,\n"
            "    version                int          not null,\n"
            "    block_header_hash      varchar(65)  not null,\n"
            "    prev_block_header_hash varchar(65)  not null,\n"
            "    merkle_root_hash       varchar(65)  not null,\n"
            "    time                   int unsigned not null,\n"
            "    nBits                  int unsigned not null,\n"
            "    nonce                  int unsigned not null,\n"
            "    primary key (block_h_id),\n"
            "    foreign key (block_h_id) references block (block_id)\n"
            ") ENGINE = %s;";
        char filtered_query[1000];
        sprintf(filtered_query, sql_query, PERSISTENCE_ENGINE, PERSISTENCE_ENGINE);
        return mysql_create_table(filtered_query);
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        g_global_block_table = g_hash_table_new(g_str_hash, g_str_equal);
        return true;
    }

    return false;
}

/**
 * Save a block in the database.
 * @param bl A block.
 * @return True for success and false otherwise.
 * @auhtor Luke E
 */
bool save_block(block *bl) {
    // Save the genesis block.
    if (get_total_number_of_blocks() == 0) {
        g_genesis_block = bl;
    }

    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        int temp_sql_query_size = 10000;

        // Save the block header in table blockheader.
        char sql_query[temp_sql_query_size];
        memset(sql_query, 0, temp_sql_query_size);
        sprintf(sql_query,
                "set @txn_count := %d;\n"
                "insert into block(block_id, txn_count)\n"
                "values (NULL, @txn_count);\n"
                "set @block_h_id = LAST_INSERT_ID();\n",
                bl->txn_count);
        if (!mysql_insert(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert block.");
            return false;
        };
        memset(sql_query, '\0', temp_sql_query_size);

        // Insert block header.
        block_header *current_header = bl->header;
        sprintf(sql_query,
                "set @version = %d;\n"
                "set @prev_block_header_hash = '%s';\n"
                "set @merkle_root_hash = '%s';\n"
                "set @block_header_hash = '%s';\n"
                "set @time = %u;\n"
                "set @nBits = %u;\n"
                "set @nonce= %u;\n"
                "insert into block_header(block_h_id, version, block_header_hash, prev_block_header_hash, merkle_root_hash, time, nBits, nonce)\n"
                "values (@block_h_id, @version, @block_header_hash, @prev_block_header_hash, @merkle_root_hash, @time, @nBits, @nonce);",
                current_header->version,
                current_header->prev_block_header_hash,
                current_header->merkle_root_hash,
                hash_block_header(current_header),
                current_header->time,
                current_header->nBits,
                current_header->nonce);
        if (!mysql_insert(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert block header.");
            return false;
        }
        memset(sql_query, '\0', temp_sql_query_size);

        // Update the block ID for all associated transactions.
        unsigned long block_id = get_block_id_in_database(bl);
        for (int i = 0; i < bl->txn_count; i++) {
            transaction *current_transaction = bl->txns[i];
            char *txid = get_transaction_txid(current_transaction);
            if (!does_transaction_exist(txid)) {
                save_transaction(current_transaction);
            }
            update_transaction_block_id(block_id, txid);
            free(txid);
        }

        return true;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        char *cur_block_hash = hash_block_header(bl->header);
        g_hash_table_insert(g_global_block_table, cur_block_hash, bl);
        return true;
    }

    return false;
}

/**
 * Get the block ID of a specific block in the MySQL database.
 * @param block The specific block.
 * @return Its block ID.
 */
unsigned long get_block_id_in_database(block *block) {
    char *header_hash = hash_block_header(block->header);
    char sql_query[1000];
    sprintf(sql_query, "select block_h_id from block_header where block_header_hash='%s';", header_hash);
    MYSQL_RES *res = mysql_read(sql_query);

    MYSQL_ROW row;
    unsigned long block_header_id;
    while ((row = mysql_fetch_row(res))) {
        block_header_id = atoi(row[0]);
    }
    free(header_hash);
    return block_header_id;
}

/**
 * Check if a block exists in the database.
 * @param block_header_hash The hash of the block header.
 * @return True for exists and false otherwise.
 * @author Luke E
 */
bool does_block_exist(char *block_header_hash) {
    char sql_query[1000];
    memset(sql_query, '\0', 1000);
    sprintf(sql_query, "select * from block_header where block_header_hash='%s';", block_header_hash);
    MYSQL_RES *res = mysql_read(sql_query);
    bool result = res->row_count > 0;
    mysql_free_result(res);
    return result;
}

/**
 * Get the block from the database by its header hash.
 * @param block_header_hash The hash of the block header.
 * @return The block.
 * @author Luke E
 */
block *get_block(char *block_header_hash) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        block *b = (block *)malloc(sizeof(block));
        memset(b, 0, sizeof(block));
        b->header = (block_header *)(malloc(sizeof(block_header)));
        memset(b->header, 0, sizeof(block_header));
        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];

        // Read block header.
        sprintf(sql_query, "select * from block_header where block_header_hash='%s';", block_header_hash);
        MYSQL_RES *res = mysql_read(sql_query);
        MYSQL_ROW row = mysql_fetch_row(res);
        unsigned long block_id;
        block_id = atoi(row[0]);
        b->header->version = atoi(row[1]);
        memcpy(b->header->prev_block_header_hash, row[2], 64);
        memcpy(b->header->merkle_root_hash, row[3], 64);
        b->header->time = atoi(row[4]);
        b->header->nBits = atoi(row[5]);
        b->header->nonce = atoi(row[6]);
        mysql_free_result(res);
        memset(sql_query, 0, temp_sql_query_size);

        // Read block.
        sprintf(sql_query, "select * from block where block_id=%lu;", block_id);
        res = mysql_read(sql_query);
        row = mysql_fetch_row(res);
        b->txn_count = atoi(row[1]);
        mysql_free_result(res);
        memset(sql_query, 0, temp_sql_query_size);

        // Read associated transactions.
        char **txids = (char **)malloc(b->txn_count * sizeof(char *));
        memset(txids, 0, b->txn_count);
        for (int i = 0; i < b->txn_count; i++) {
            txids[i] = (char *)malloc(65);
            memset(txids[i], 0, 65);
        }

        sprintf(sql_query, "select txid from transaction where block_id=%lu;", block_id);
        res = mysql_read(sql_query);
        int tx_count = 0;
        while ((row = mysql_fetch_row(res))) {
            memcpy(txids[tx_count], row[0], 64);
            tx_count++;
        }
        b->txns = (transaction **)malloc(b->txn_count * sizeof(transaction *));
        for (int i = 0; i < b->txn_count; i++) {
            b->txns[i] = get_transaction(txids[i]);
        }

        mysql_free_result(res);
        memset(sql_query, 0, temp_sql_query_size);
        for (int i = 0; i < b->txn_count; i++) free(txids[i]);
        free(txids);

        return b;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        return g_hash_table_lookup(g_global_block_table, block_header_hash);
    }
}

/**
 * Get the genesis block in the system.
 * @return The genesis block.
 * @author Ing Tian
 */
block *get_genesis_block() {
    if (g_genesis_block != NULL) {
        return g_genesis_block;
    }

    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query = "select block_header_hash from block_header where block_h_id=1;";
        MYSQL_RES *res = mysql_read(sql_query);

        MYSQL_ROW row;
        char genesis_block_header_id[65];
        genesis_block_header_id[64] = '\0';
        while ((row = mysql_fetch_row(res))) {
            strcpy(genesis_block_header_id, row[0]);
        }

        mysql_free_result(res);
        block *genesis_block = get_block(genesis_block_header_id);
        g_genesis_block = genesis_block;
        return g_genesis_block;
    }

    return NULL;
}

/**
 * Destroy the block persistence layer
 * by destroying all the tables.
 * @param db_name The name of the MySQL database to use.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool destroy_block_persistence(char *db_name) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query =
            "use %s;\n"
            "drop table if exists block_header;\n"
            "drop table if exists block;\n";
        char filtered_sql_query[1000];
        sprintf(filtered_sql_query, sql_query, db_name);
        return mysql_delete_table(filtered_sql_query);
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        g_hash_table_foreach(g_global_block_table, free_g_global_block_table_entry, NULL);
        g_hash_table_destroy(g_global_block_table);
        return true;
    }
    return false;
}

/**
 * Get total number of blocks in the system.
 * @return The total number of blocks in the system.
 * @author Ing Tian
 */
unsigned int get_total_number_of_blocks() {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query = "select count(*) as count from block;";
        MYSQL_RES *res = mysql_read(sql_query);

        // Read number.
        MYSQL_ROW row;
        unsigned int answer = 0;
        while ((row = mysql_fetch_row(res))) {
            answer = atoi(row[0]);
        }

        mysql_free_result(res);

        return answer;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        return g_hash_table_size(g_global_block_table);
    }

    return -1;
};

/**
 * Get last inserted block from the database.
 * @return A block.
 * @author Ing Tian
 */
block *get_last_inserted_block() {
    unsigned int last_block_idx = get_total_number_of_blocks();

    char sql_query[1000];
    sprintf(sql_query, "select block_header_hash from block_header where block_h_id=%u;", last_block_idx);
    MYSQL_RES *res = mysql_read(sql_query);

    MYSQL_ROW row;
    char temp_header_id[65];
    temp_header_id[64] = '\0';
    while ((row = mysql_fetch_row(res))) {
        strcpy(temp_header_id, row[0]);
    }

    mysql_free_result(res);
    block *blk = get_block(temp_header_id);
    return blk;
}
