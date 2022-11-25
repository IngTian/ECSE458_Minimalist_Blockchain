#include "block_persistence.h"

#include <string.h>

#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "block_persistence"

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
    char *sql_query =
        "CREATE TABLE block\n"
        "(\n"
        "block_id  int auto_increment,\n"
        "txn_count int unsigned not null,\n"
        "primary key (block_id)\n"
        ");\n"
        "\n"
        "CREATE TABLE blockheader\n"
        "(\n"
        "block_h_id             int          not null,\n"
        "version                int          not null,\n"
        "prev_block_header_hash BLOB         not null,\n"
        "merkle_root_hash       BLOB         not null,\n"
        "time                   int unsigned not null,\n"
        "nBits                  int unsigned not null,\n"
        "nonce                  int unsigned not null,\n"
        "primary key (block_h_id),\n"
        "foreign key (block_h_id) references block (block_id)\n"
        ");\n";
    return mysql_create_table(sql_query);
}

/**
 * Save a block in the database.
 * @param bl A block.
 * @return True for success and false otherwise.
 * @auhtor Luke E
 */
bool save_block(block *bl) {
    int temp_sql_query_size = 10000;

    // Save the block header in table blockheader.
    char sql_query[temp_sql_query_size];
    sprintf(sql_query,
            "set @txn_count := %d;\n"
            "insert into block(block_id, txn_count)\n"
            "values (NULL, @txn_count);\n",
            bl->txn_count);
    if (!mysql_insert(sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert block.");
        return false;
    };
    memset(sql_query, '\0', temp_sql_query_size);

    // Insert block header.

    block_header current_header = bl->header;
    sprintf(sql_query,
            "set @block_h_id = LAST_INSERT_ID();\n"
            "set @version = %d;\n"
            "set @prev_block_header_hash = 0x%d;\n"
            "set @merkle_root_hash = 0x%d;\n"
            "set @time = %u;\n"
            "set @nBits = %u;\n"
            "set @nonce= %u;\n"
            "insert into blockheader(block_h_id, version, prev_block_header_hash, merkle_root_hash, time, nBits, nonce)\n"
            "values (@block_h_id, @version, @prev_block_header_hash, @merkle_root_hash, @time, @nBits, @nonce);",

            current_header.version,
            current_header.prev_block_header_hash,
            current_header.merkle_root_hash,
            current_header.time,
            current_header.nBits,
            current_header.nonce
            );
    if (!mysql_insert(sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert block header.");
        return false;
    }
    memset(sql_query, '\0', temp_sql_query_size);

    return true;
}

/**
 * Destroy the block persistence layer
 * by destroying all the tables.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool destroy_block_persistence() {
    char *sql_query =
        "drop table blockheader;\n"
        "drop table block;\n" ;
    return mysql_delete_table(sql_query);
}