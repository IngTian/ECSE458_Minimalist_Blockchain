#include "transaction_persistence.h"

#include <string.h>

#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "transaction_persistence"

/*
 * -----------------------------------------------------------
 * APIs
 * -----------------------------------------------------------
 */

/**
 * Initialize the persistence layer by creating tables
 * in the database.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool initialize_transaction_persistence() {
    char *sql_query =
        "create table transaction\n"
        "(\n"
        "    id           int auto_increment,\n"
        "    txid         BLOB         not null,\n"
        "    version      int          not null,\n"
        "    tx_in_count  int unsigned not null,\n"
        "    tx_out_count int unsigned not null,\n"
        "    lock_time    int unsigned not null,\n"
        "    primary key (id)\n"
        ");\n"
        "\n"
        "create table transaction_output\n"
        "(\n"
        "    id              int auto_increment,\n"
        "    value           bigint       not null,\n"
        "    pk_script_bytes int unsigned not null,\n"
        "    pk_script       BLOB         not null,\n"
        "    transaction_id  int          not null,\n"
        "    primary key (id),\n"
        "    constraint foreign key (transaction_id) references transaction (id)\n"
        ");\n"
        "\n"
        "create table transaction_input\n"
        "(\n"
        "    id               int auto_increment,\n"
        "    script_bytes     int unsigned not null,\n"
        "    signature_script BLOB         not null,\n"
        "    sequence         int unsigned not null,\n"
        "    transaction_id   int          not null,\n"
        "    primary key (id),\n"
        "    foreign key (transaction_id) references transaction (id)\n"
        ");\n"
        "\n"
        "create table transaction_outpoint\n"
        "(\n"
        "    id                   int auto_increment,\n"
        "    hash                 BLOB         not null,\n"
        "    idx                  int unsigned not null,\n"
        "    transaction_input_id int          not null,\n"
        "    primary key (id),\n"
        "    foreign key (transaction_input_id) references transaction_input (id)\n"
        ");";
    return mysql_create_table(sql_query);
}

/**
 * Save a transaction in the database.
 * @param tx A transaction.
 * @return True for success and false otherwise.
 * @auhtor Ing Tian
 */
bool save_transaction(transaction *tx) {
    char *txid = get_transaction_txid(tx);
    int temp_sql_query_size = 10000;

    // Save the transaction in table transaction.
    char sql_query[temp_sql_query_size];
    sprintf(sql_query,
            "set @txid := 0x%s;\n"
            "set @version := %d;\n"
            "set @tx_in_count := %u;\n"
            "set @tx_out_count := %u;\n"
            "set @lock_time := %u;\n"
            "insert into transaction (id, txid, version, tx_in_count, tx_out_count, lock_time)\n"
            "values (NULL, @txid, @version, @tx_in_count, @tx_out_count, @lock_time);\n"
            "set @transaction_auto_id := LAST_INSERT_ID();",
            txid,
            tx->version,
            tx->tx_in_count,
            tx->tx_out_count,
            tx->lock_time);
    if (!mysql_insert(sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert transaction.");
        return false;
    };
    free(txid);
    memset(sql_query, '\0', temp_sql_query_size);

    // Insert transaction outputs.
    for (int i = 0; i < tx->tx_out_count; i++) {
        transaction_output current_output = tx->tx_outs[i];
        char *pk_script_hex = convert_char_hexadecimal(current_output.pk_script, current_output.pk_script_bytes);
        sprintf(sql_query,
                "set @value = %ld;\n"
                "set @pk_script_bytes = %d;\n"
                "set @pk_script = 0x%s;\n"
                "insert into transaction_output (id, value, pk_script_bytes, pk_script, transaction_id)\n"
                "values (NULL, @value, @pk_script_bytes, @pk_script, @transaction_auto_id);",
                current_output.value,
                current_output.pk_script_bytes,
                pk_script_hex);
        if (!mysql_insert(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert output.");
            return false;
        }
        memset(sql_query, '\0', temp_sql_query_size);
        free(pk_script_hex);
    }

    // Insert transaction inputs.
    for (int i = 0; i < tx->tx_in_count; i++) {
        transaction_input current_input = tx->tx_ins[i];
        char *signature_script_hex = convert_char_hexadecimal(current_input.signature_script, current_input.script_bytes);
        transaction_outpoint current_outpoint = current_input.previous_outpoint;
        sprintf(sql_query,
                "set @script_bytes = %u;\n"
                "set @signature_script = 0x%s;\n"
                "set @sequence = %u;\n"
                "set @hash = 0x%s;\n"
                "set @index = %u;\n"
                "insert into transaction_input (id, script_bytes, signature_script, sequence, transaction_id)\n"
                "values (NULL, @script_bytes, @signature_script, @sequence, @transaction_auto_id);\n"
                "set @transaction_input_auto_id := LAST_INSERT_ID();\n"
                "insert into transaction_outpoint (id, hash, idx, transaction_input_id)\n"
                "values (NULL, @hash, @index, @transaction_input_auto_id);",
                current_input.script_bytes,
                signature_script_hex,
                current_input.sequence,
                current_outpoint.hash,
                current_outpoint.index);
        if (!mysql_insert(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert input.");
            return false;
        }
        free(signature_script_hex);
    }

    return true;
}

transaction *get_transaction(char *txid);

/**
 * Destroy the transaction persistence layer
 * by destroying all the tables.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool destroy_transaction_persistence() {
    char *sql_query =
        "drop table transaction_outpoint;\n"
        "drop table transaction_input;\n"
        "drop table transaction_output;\n"
        "drop table transaction;";
    return mysql_delete_table(sql_query);
}