#include "transaction_persistence.h"

#include <glib.h>
#include <mysql.h>
#include <string.h>

#include "utils/constants.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "transaction_persistence"

static GHashTable *g_global_transaction_table;     // The global transaction table, mapping TXID to transaction.
static GHashTable *g_utxo;                         // Unspent Transaction Output. mapping each transaction output to its value left.
static transaction *g_genesis_transaction = NULL;  // The genesis transaction.

/*
 * -----------------------------------------------------------
 * Helper methods.
 * -----------------------------------------------------------
 */
void free_transaction_table_key(void *key) { free(key); }

void free_transaction_table_val(void *val) { destroy_transaction(val); }

void free_utxo_table_key(void *key) { free(key); }

void free_utxo_table_val(void *val) { free(val); }

void print_utxo_entry(void *h, void *v, void *user_data) {
    char *hash = (char *)h;
    long int *value = (long int *)v;
    printf("ID: %s VAL: %ld\n", hash, *value);
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
 * @author Ing Tian
 */
bool initialize_transaction_persistence() {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query =
            "create table if not exists transaction\n"
            "(\n"
            "    id           int auto_increment,\n"
            "    txid         char(64)     not null unique,\n"
            "    version      int          not null,\n"
            "    tx_in_count  int unsigned not null,\n"
            "    tx_out_count int unsigned not null,\n"
            "    lock_time    int unsigned not null,\n"
            "    block_id     int          not null default 0,\n"
            "    primary key (id)\n"
            ") ENGINE = %s;\n"
            "\n"
            "create table if not exists transaction_output\n"
            "(\n"
            "    id              int auto_increment,\n"
            "    value           bigint         not null,\n"
            "    pk_script_bytes int unsigned   not null,\n"
            "    pk_script       varchar(10000) not null,\n"
            "    transaction_id  int            not null,\n"
            "    primary key (id),\n"
            "    constraint foreign key (transaction_id) references transaction (id)\n"
            ") ENGINE = %s;\n"
            "\n"
            "create table if not exists transaction_input\n"
            "(\n"
            "    id               int auto_increment,\n"
            "    script_bytes     int unsigned   not null,\n"
            "    signature_script varchar(10000) not null,\n"
            "    sequence         int unsigned   not null,\n"
            "    transaction_id   int            not null,\n"
            "    primary key (id),\n"
            "    foreign key (transaction_id) references transaction (id)\n"
            ") ENGINE = %s;\n"
            "\n"
            "create table if not exists transaction_outpoint\n"
            "(\n"
            "    id                   int auto_increment,\n"
            "    hash                 char(64)     not null,\n"
            "    idx                  int unsigned not null,\n"
            "    transaction_input_id int          not null,\n"
            "    primary key (id),\n"
            "    foreign key (transaction_input_id) references transaction_input (id)\n"
            ") ENGINE = %s;\n"
            "\n"
            "create table if not exists utxo\n"
            "(\n"
            "    id    int auto_increment,\n"
            "    hash  char(64) not null,\n"
            "    value bigint   not null,\n"
            "    primary key (id)\n"
            ") ENGINE = %s;";
        char filtered_query[10000];
        sprintf(filtered_query, sql_query, PERSISTENCE_ENGINE, PERSISTENCE_ENGINE, PERSISTENCE_ENGINE, PERSISTENCE_ENGINE, PERSISTENCE_ENGINE);
        return mysql_create_table(filtered_query);
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        g_global_transaction_table = g_hash_table_new_full(g_str_hash, g_str_equal, free_transaction_table_key, free_transaction_table_val);
        g_utxo = g_hash_table_new_full(g_str_hash, g_str_equal, free_utxo_table_key, free_utxo_table_val);
    }

    return false;
}

/**
 * Save a transaction in the database.
 * @param tx A transaction.
 * @return True for success and false otherwise.
 * @auhtor Ing Tian
 */
bool save_transaction(transaction *tx) {
    // Save the genesis transaction.
    if (get_total_number_of_transactions() == 0) {
        g_genesis_transaction = tx;
    }

    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *txid = get_transaction_txid(tx);
        int temp_sql_query_size = 10000;

        // Save the transaction in table transaction.
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);
        sprintf(sql_query,
                "set @txid := '%s';\n"
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
                    "set @pk_script = '%s';\n"
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
                    "set @signature_script = '%s';\n"
                    "set @sequence = %u;\n"
                    "set @hash = '%s';\n"
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
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        char *txid = get_transaction_txid(tx);
        g_hash_table_insert(g_global_transaction_table, txid, tx);
        return true;
    }

    return false;
}

/**
 * Save a utxo entry.
 * @param key The key.
 * @param value The value.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool save_utxo_entry(char *key, long int *value) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);
        sprintf(sql_query,
                "set @hash := '%s';\n"
                "set @value := %ld;\n"
                "insert into utxo (id, hash, value)\n"
                "values (NULL, @hash, @value);\n",
                key,
                *value);
        if (!mysql_insert(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert UTXO entry.");
            return false;
        } else {
            return true;
        };
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        g_hash_table_insert(g_utxo, key, value);
        return true;
    }

    return false;
}

/**
 * Remove a UTXO entry.
 * @param key A key.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool remove_utxo_entry(char *key) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);
        sprintf(sql_query, "delete from utxo where hash='%s';\n", key);
        if (!mysql_delete(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to delete UTXO entry.");
            return false;
        } else {
            return true;
        };
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        g_hash_table_remove(g_utxo, key);
        return true;
    }

    return false;
}

/**
 * Print UTXO inside the system.
 * @author Ing Tian
 */
void print_utxo() {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        return;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        printf("**************************** UTXO *****************************\n");
        g_hash_table_foreach(g_utxo, print_utxo_entry, NULL);
        printf("\n");
    }
}

/**
 * Update the block ID in the transaction.
 * @param block_id The block ID to update to.
 * @param txid The transaction ID.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool update_transaction_block_id(unsigned long block_id, char *txid) {
    char sql_query[1000];
    memset(sql_query, '\0', 1000);
    sprintf(sql_query, "update transaction set block_id=%lu where txid='%s';", block_id, txid);
    if (!mysql_update(sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to update block ID (%d) for a transaction (%s).", block_id, txid);
        return false;
    }
    return true;
}

/**
 * Get a transaction from the database by its txid.
 * @param txid The transaction ID.
 * @return A transaction.
 * @author Ing Tian
 */
transaction *get_transaction(char *txid) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        transaction *tx = (transaction *)malloc(sizeof(transaction));

        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);

        sprintf(sql_query, "select * from transaction where txid='%s';", txid);
        MYSQL_RES *res = mysql_read(sql_query);

        // Read transaction.
        MYSQL_ROW row;
        int transaction_auto_id;
        while ((row = mysql_fetch_row(res))) {
            transaction_auto_id = atoi(row[0]);
            tx->version = atoi(row[2]);
            tx->tx_in_count = atoi(row[3]);
            tx->tx_out_count = atoi(row[4]);
            tx->lock_time = atoi(row[5]);
        }
        tx->tx_ins = (transaction_input *)malloc(tx->tx_in_count * sizeof(transaction_input));
        memset(tx->tx_ins, 0, tx->tx_in_count * sizeof(transaction_input));
        tx->tx_outs = (transaction_output *)malloc(tx->tx_out_count * sizeof(transaction_output));
        memset(tx->tx_outs, 0, tx->tx_out_count * sizeof(transaction_output));
        mysql_free_result(res);
        memset(sql_query, '\0', temp_sql_query_size);

        // Read transaction outputs.
        sprintf(sql_query, "select * from transaction_output where transaction_id=%d;", transaction_auto_id);
        res = mysql_read(sql_query);
        int output_idx = 0;
        while ((row = mysql_fetch_row(res))) {
            transaction_output *current_output = &tx->tx_outs[output_idx];
            current_output->value = atoi(row[1]);
            current_output->pk_script_bytes = atoi(row[2]);
            current_output->pk_script = (char *)malloc(current_output->pk_script_bytes);
            char *converted_pk_script = convert_hex_back_to_data_array(row[3]);
            memcpy(current_output->pk_script, converted_pk_script, current_output->pk_script_bytes);
            free(converted_pk_script);
            output_idx++;
        }
        mysql_free_result(res);
        memset(sql_query, '\0', temp_sql_query_size);

        // Read transaction inputs.
        sprintf(sql_query, "select * from transaction_input where transaction_id=%d;", transaction_auto_id);
        res = mysql_read(sql_query);
        int input_idx = 0;
        int outpoint_input_ids[tx->tx_in_count];
        memset(outpoint_input_ids, 0, tx->tx_in_count);
        while ((row = mysql_fetch_row(res))) {
            transaction_input *current_input = &tx->tx_ins[input_idx];
            outpoint_input_ids[input_idx] = atoi(row[0]);
            current_input->script_bytes = atoi(row[1]);
            current_input->signature_script = (char *)malloc(current_input->script_bytes);
            char *converted_signature_script = convert_hex_back_to_data_array(row[2]);
            memcpy(current_input->signature_script, converted_signature_script, current_input->script_bytes);
            free(converted_signature_script);
            current_input->sequence = atoi(row[3]);
            input_idx++;
        }
        mysql_free_result(res);
        memset(sql_query, '\0', temp_sql_query_size);

        // Read transaction input's outpoints.
        for (int outpoint_idx = 0; outpoint_idx < tx->tx_in_count; outpoint_idx++) {
            sprintf(sql_query, "select * from transaction_outpoint where transaction_input_id=%d;", outpoint_input_ids[outpoint_idx]);
            res = mysql_read(sql_query);

            row = mysql_fetch_row(res);
            transaction_outpoint *current_outpoint = &tx->tx_ins[outpoint_idx].previous_outpoint;
            current_outpoint->index = atoi(row[2]);
            memset(current_outpoint->hash, '\0', 65);
            memcpy(current_outpoint->hash, row[1], 64);

            mysql_free_result(res);
            memset(sql_query, '\0', temp_sql_query_size);
        }

        return tx;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        return g_hash_table_lookup(g_global_transaction_table, txid);
    }

    return NULL;
}

/**
 * Get the genesis transaction.
 * @return The genesis transaction.
 * @author Ing Tian
 */
transaction *get_genesis_transaction() {
    if (g_genesis_transaction != NULL) {
        return g_genesis_transaction;
    }

    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query = "select txid from transaction where id=1;";
        MYSQL_RES *res = mysql_read(sql_query);

        MYSQL_ROW row;
        char genesis_txid[65];
        genesis_txid[64] = '\0';
        while ((row = mysql_fetch_row(res))) {
            strcpy(genesis_txid, row[0]);
        }

        mysql_free_result(res);
        transaction *genesis_transaction = get_transaction(genesis_txid);
        g_genesis_transaction = genesis_transaction;
        return genesis_transaction;
    }

    return NULL;
}

/**
 * Determines if a transaction exists.
 * @param txid The transaction ID.
 * @return True if the transaction exists and false otherwise.
 * @author Ing Tian
 */
bool does_transaction_exist(char *txid) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char sql_query[1000];
        memset(sql_query, '\0', 1000);
        sprintf(sql_query, "select * from transaction where txid='%s';", txid);
        MYSQL_RES *res = mysql_read(sql_query);
        bool result = res->row_count > 0;
        mysql_free_result(res);
        return result;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        return g_hash_table_contains(g_global_transaction_table, txid);
    }

    return false;
};

/**
 * Check if a key exists in UTXO.
 * @param key A UTXO key.
 * @return True for exists and false otherwise.
 * @author Ing Tian
 */
bool does_utxo_entry_exist(char *key) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char sql_query[1000];
        memset(sql_query, '\0', 1000);
        sprintf(sql_query, "select * from utxo where hash='%s';\n", key);
        MYSQL_RES *res = mysql_read(sql_query);
        bool result = res->row_count > 0;
        mysql_free_result(res);
        return result;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        return g_hash_table_contains(g_utxo, key);
    }

    return false;
}

/**
 * Destroy the transaction persistence layer
 * by destroying all the tables.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool destroy_transaction_persistence() {
    bool res = false;
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query =
            "drop table if exists transaction_outpoint;\n"
            "drop table if exists transaction_input;\n"
            "drop table if exists transaction_output;\n"
            "drop table if exists transaction;";
        res = mysql_delete_table(sql_query);
        if (!res) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to delete tables.");
        }
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        g_hash_table_remove_all(g_utxo);
        g_hash_table_destroy(g_utxo);
        g_hash_table_remove_all(g_global_transaction_table);
        g_hash_table_destroy(g_global_transaction_table);
        res = true;
    }

    return res;
}

/**
 * Get the total number of transactions in the system.
 * @return The total number of transactions in the system.
 * @author Ing Tian
 */
unsigned int get_total_number_of_transactions() {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query = "select count(*) as count from transaction;";
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
        return g_hash_table_size(g_global_transaction_table);
    }
    return -1;
}

/**
 * Get last inserted block from the database.
 * @return A block.
 * @author Ing Tian
 */
transaction *get_last_inserted_transaction() {
    unsigned int last_tx_idx = get_total_number_of_transactions();

    char sql_query[1000];
    sprintf(sql_query, "select txid from transaction where id=%u;", last_tx_idx);
    MYSQL_RES *res = mysql_read(sql_query);

    MYSQL_ROW row;
    char temp_txid[65];
    temp_txid[64] = '\0';
    while ((row = mysql_fetch_row(res))) {
        strcpy(temp_txid, row[0]);
    }

    mysql_free_result(res);
    transaction *tx = get_transaction(temp_txid);
    return tx;
}
