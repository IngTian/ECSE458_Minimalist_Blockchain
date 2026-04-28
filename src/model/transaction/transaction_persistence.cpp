#include "transaction_persistence.h"

#include <glib.h>
#include <mysql/mysql.h>
#include <stdint.h>
#include <string.h>

#include <array>
#include <cstring>
#include <memory>
#include <optional>

#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "transaction_persistence"

static GHashTable *g_global_transaction_table;     // The global transaction table, mapping TXID to transaction.
static GHashTable *g_utxo;                         // Unspent Transaction Output. mapping each transaction output to its value left.
static transaction *g_genesis_transaction = NULL;  // The genesis transaction.
static uint8_t g_genesis_txid[32];                 // Cached TXID of the genesis transaction.
static bool g_genesis_txid_set = false;

// Single-entry cache for the last non-genesis transaction fetched from MySQL.
// Chained workloads (verify, finalize) re-fetch the same prev_txid 3+ times in
// a row; this cache turns those repeats into a memcmp. The unique_ptr owns the
// transaction with destroy_transaction as the deleter — eviction is automatic
// when the entry is reassigned or the optional is reset.
struct transaction_deleter {
    void operator()(transaction *t) const noexcept {
        if (t) destroy_transaction(t);
    }
};
using owned_transaction = std::unique_ptr<transaction, transaction_deleter>;

struct cache_entry {
    std::array<uint8_t, 32> txid;
    owned_transaction tx;
};

static std::optional<cache_entry> g_last_fetched;

static void invalidate_get_transaction_cache(void) noexcept {
    g_last_fetched.reset();
}

/* Allocates a deep copy of `src` so the cache can own/free it independently of the caller. */
static transaction *deep_copy_transaction(const transaction *src) {
    transaction *dst = (transaction *)malloc(sizeof(transaction));
    dst->version = src->version;
    dst->tx_in_count = src->tx_in_count;
    dst->tx_out_count = src->tx_out_count;
    dst->lock_time = src->lock_time;
    dst->tx_ins = (transaction_input *)malloc(src->tx_in_count * sizeof(transaction_input));
    dst->tx_outs = (transaction_output *)malloc(src->tx_out_count * sizeof(transaction_output));
    for (unsigned int i = 0; i < src->tx_in_count; i++) {
        dst->tx_ins[i].script_bytes = src->tx_ins[i].script_bytes;
        dst->tx_ins[i].sequence = src->tx_ins[i].sequence;
        dst->tx_ins[i].previous_outpoint = src->tx_ins[i].previous_outpoint;
        dst->tx_ins[i].signature_script = (char *)malloc(src->tx_ins[i].script_bytes);
        memcpy(dst->tx_ins[i].signature_script, src->tx_ins[i].signature_script, src->tx_ins[i].script_bytes);
    }
    for (unsigned int i = 0; i < src->tx_out_count; i++) {
        dst->tx_outs[i].value = src->tx_outs[i].value;
        dst->tx_outs[i].pk_script_bytes = src->tx_outs[i].pk_script_bytes;
        dst->tx_outs[i].pk_script = (char *)malloc(src->tx_outs[i].pk_script_bytes);
        memcpy(dst->tx_outs[i].pk_script, src->tx_outs[i].pk_script, src->tx_outs[i].pk_script_bytes);
    }
    return dst;
}

/*
 * -----------------------------------------------------------
 * Helper methods.
 * -----------------------------------------------------------
 */
void free_transaction_table_key(void *key) { free(key); }

void free_transaction_table_val(void *val) { destroy_transaction((transaction *)val); }

void free_utxo_table_key(void *key) { free(key); }

void free_utxo_table_val(void *val) { free(val); }

static guint binary_hash32(gconstpointer key) {
    const uint8_t *d = (const uint8_t *)key;
    guint h = 5381;
    for (int i = 0; i < 32; i++) h = (h << 5) + h + d[i];
    return h;
}
static gboolean binary_equal32(gconstpointer a, gconstpointer b) {
    return memcmp(a, b, 32) == 0;
}

void print_utxo_entry(void *h, void *v, void *user_data) {
    char *hex = hash_to_hex((const uint8_t *)h);
    long int *value = (long int *)v;
    general_log(LOG_SCOPE, LOG_DEBUG, "ID: %s VAL: %ld", hex, *value);
    free(hex);
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
            "set max_heap_table_size = 1024*1024*1024*2;\n"
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
        g_global_transaction_table = g_hash_table_new_full(binary_hash32, binary_equal32, free_transaction_table_key, free_transaction_table_val);
        g_utxo = g_hash_table_new_full(binary_hash32, binary_equal32, free_utxo_table_key, free_utxo_table_val);
    }

    /* In MYSQL mode, also keep g_utxo as a fresh in-memory mirror so
       does_utxo_entry_exist can short-circuit the SELECT. NOTE: this assumes a
       fresh start (tables wiped or first init); resume scenarios would need a
       startup load. */
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL && g_utxo == NULL) {
        g_utxo = g_hash_table_new_full(binary_hash32, binary_equal32, free, free);
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
    unsigned int current_tx_size = get_total_number_of_transactions();
    if (current_tx_size == 0) {
        g_genesis_transaction = tx;
        uint8_t *txid_for_cache = get_transaction_txid(tx);
        memcpy(g_genesis_txid, txid_for_cache, 32);
        free(txid_for_cache);
        g_genesis_txid_set = true;
    }

    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        uint8_t *txid_bin = get_transaction_txid(tx);
        char *txid = hash_to_hex(txid_bin);
        free(txid_bin);

        /* Build one big multi-statement query that covers transaction + all outputs + all inputs/outpoints,
           so save_transaction does a single mysql_query roundtrip. */
        size_t buf_cap = 4096 + (size_t)tx->tx_out_count * 1024 + (size_t)tx->tx_in_count * 2048;
        char *buf = (char *)malloc(buf_cap);
        size_t off = 0;
        off += snprintf(buf + off, buf_cap - off,
                        "insert into transaction (id, txid, version, tx_in_count, tx_out_count, lock_time)"
                        " values (NULL, '%s', %d, %u, %u, %u);",
                        txid, tx->version, tx->tx_in_count, tx->tx_out_count, tx->lock_time);
        free(txid);

        unsigned int related_tx_idx = current_tx_size + 1;
        for (int i = 0; i < tx->tx_out_count; i++) {
            transaction_output current_output = tx->tx_outs[i];
            char *pk_script_hex = convert_char_hexadecimal(current_output.pk_script, current_output.pk_script_bytes);
            off += snprintf(buf + off, buf_cap - off,
                            "insert into transaction_output (id, value, pk_script_bytes, pk_script, transaction_id)"
                            " values (NULL, %ld, %d, '%s', %u);",
                            current_output.value, current_output.pk_script_bytes, pk_script_hex, related_tx_idx);
            free(pk_script_hex);
        }

        for (int i = 0; i < tx->tx_in_count; i++) {
            transaction_input current_input = tx->tx_ins[i];
            char *signature_script_hex = convert_char_hexadecimal(current_input.signature_script, current_input.script_bytes);
            char *outpoint_hash_hex = hash_to_hex(current_input.previous_outpoint.hash);
            off += snprintf(buf + off, buf_cap - off,
                            "insert into transaction_input (id, script_bytes, signature_script, sequence, transaction_id)"
                            " values (NULL, %u, '%s', %u, %u);"
                            "insert into transaction_outpoint (id, hash, idx, transaction_input_id)"
                            " values (NULL, '%s', %u, LAST_INSERT_ID());",
                            current_input.script_bytes, signature_script_hex, current_input.sequence, related_tx_idx,
                            outpoint_hash_hex, current_input.previous_outpoint.index);
            free(signature_script_hex);
            free(outpoint_hash_hex);
        }

        bool ok = mysql_insert(buf);
        free(buf);
        if (!ok) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to save transaction.");
            return false;
        }
        return true;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        uint8_t *txid = get_transaction_txid(tx);
        g_hash_table_insert(g_global_transaction_table, txid, tx);
        return true;
    }

    return false;
}

/**
 * Persist a finalized transaction together with its UTXO updates in a single
 * mysql_query roundtrip: insert transaction + outputs + inputs/outpoints,
 * delete spent UTXOs, insert new UTXOs.
 */
bool commit_finalized_transaction(transaction *tx) {
    if (!g_genesis_txid_set) {
        g_genesis_transaction = tx;
        uint8_t *txid_for_cache = get_transaction_txid(tx);
        memcpy(g_genesis_txid, txid_for_cache, 32);
        free(txid_for_cache);
        g_genesis_txid_set = true;
    }

    if (PERSISTENCE_MODE != PERSISTENCE_MYSQL) {
        if (!save_transaction(tx)) return false;
        uint8_t *txid = get_transaction_txid(tx);
        for (int i = 0; i < tx->tx_in_count; i++) {
            uint8_t *outpoint_hash = hash_transaction_outpoint(&tx->tx_ins[i].previous_outpoint);
            remove_utxo_entry(outpoint_hash);
            free(outpoint_hash);
        }
        for (int i = 0; i < tx->tx_out_count; i++) {
            long int *value = (long int *)malloc(sizeof(long int));
            *value = tx->tx_outs[i].value;
            transaction_outpoint outpoint;
            memcpy(outpoint.hash, txid, 32);
            outpoint.index = i;
            uint8_t *outpoint_hash = hash_transaction_outpoint(&outpoint);
            save_utxo_entry(outpoint_hash, value);
        }
        free(txid);
        return true;
    }

    uint8_t *txid_bin = get_transaction_txid(tx);
    char *txid_hex = hash_to_hex(txid_bin);

    size_t buf_cap = 8192 + (size_t)tx->tx_out_count * 1536 + (size_t)tx->tx_in_count * 2560;
    char *buf = (char *)malloc(buf_cap);
    size_t off = 0;

    off += snprintf(buf + off, buf_cap - off,
                    "insert into transaction (id, txid, version, tx_in_count, tx_out_count, lock_time)"
                    " values (NULL, '%s', %d, %u, %u, %u);"
                    "set @tx_auto_id := LAST_INSERT_ID();",
                    txid_hex, tx->version, tx->tx_in_count, tx->tx_out_count, tx->lock_time);

    for (int i = 0; i < tx->tx_out_count; i++) {
        transaction_output current_output = tx->tx_outs[i];
        char *pk_script_hex = convert_char_hexadecimal(current_output.pk_script, current_output.pk_script_bytes);
        off += snprintf(buf + off, buf_cap - off,
                        "insert into transaction_output (id, value, pk_script_bytes, pk_script, transaction_id)"
                        " values (NULL, %ld, %d, '%s', @tx_auto_id);",
                        current_output.value, current_output.pk_script_bytes, pk_script_hex);
        free(pk_script_hex);
    }

    for (int i = 0; i < tx->tx_in_count; i++) {
        transaction_input current_input = tx->tx_ins[i];
        char *signature_script_hex = convert_char_hexadecimal(current_input.signature_script, current_input.script_bytes);
        char *outpoint_hash_hex = hash_to_hex(current_input.previous_outpoint.hash);
        off += snprintf(buf + off, buf_cap - off,
                        "insert into transaction_input (id, script_bytes, signature_script, sequence, transaction_id)"
                        " values (NULL, %u, '%s', %u, @tx_auto_id);"
                        "insert into transaction_outpoint (id, hash, idx, transaction_input_id)"
                        " values (NULL, '%s', %u, LAST_INSERT_ID());",
                        current_input.script_bytes, signature_script_hex, current_input.sequence,
                        outpoint_hash_hex, current_input.previous_outpoint.index);
        free(signature_script_hex);
        free(outpoint_hash_hex);
    }

    /* Spend the inputs from the UTXO set (DB + in-memory mirror). */
    for (int i = 0; i < tx->tx_in_count; i++) {
        uint8_t *outpoint_hash = hash_transaction_outpoint(&tx->tx_ins[i].previous_outpoint);
        char *spent_hex = hash_to_hex(outpoint_hash);
        off += snprintf(buf + off, buf_cap - off, "delete from utxo where hash='%s';", spent_hex);
        free(spent_hex);
        if (g_utxo != NULL) g_hash_table_remove(g_utxo, outpoint_hash);
        free(outpoint_hash);
    }

    /* Insert new UTXOs for each output (DB + in-memory mirror). */
    for (int i = 0; i < tx->tx_out_count; i++) {
        transaction_outpoint outpoint;
        memcpy(outpoint.hash, txid_bin, 32);
        outpoint.index = i;
        uint8_t *outpoint_hash = hash_transaction_outpoint(&outpoint);
        char *new_hex = hash_to_hex(outpoint_hash);
        off += snprintf(buf + off, buf_cap - off,
                        "insert into utxo (id, hash, value) values (NULL, '%s', %ld);",
                        new_hex, tx->tx_outs[i].value);
        free(new_hex);
        if (g_utxo != NULL) {
            long int *cached_val = (long int *)malloc(sizeof(long int));
            *cached_val = tx->tx_outs[i].value;
            g_hash_table_insert(g_utxo, outpoint_hash, cached_val);
        } else {
            free(outpoint_hash);
        }
    }

    free(txid_hex);

    bool ok = mysql_insert(buf);
    free(buf);
    if (!ok) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to commit finalized transaction.");
        free(txid_bin);
        return false;
    }

    // Cache the just-committed tx so the next chained lookup avoids re-fetching from MySQL.
    // optional::emplace replaces any existing entry; the previous unique_ptr is destroyed first.
    cache_entry new_entry;
    std::memcpy(new_entry.txid.data(), txid_bin, 32);
    new_entry.tx.reset(deep_copy_transaction(tx));
    g_last_fetched.emplace(std::move(new_entry));
    free(txid_bin);
    return true;
}

/**
 * Save a utxo entry.
 * @param key The key.
 * @param value The value.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool save_utxo_entry(uint8_t *key, long int *value) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *key_hex = hash_to_hex(key);
        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);
        sprintf(sql_query,
                "set @hash := '%s';\n"
                "set @value := %ld;\n"
                "insert into utxo (id, hash, value)\n"
                "values (NULL, @hash, @value);\n",
                key_hex,
                *value);
        free(key_hex);
        if (!mysql_insert(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert UTXO entry.");
            return false;
        }
        if (g_utxo != NULL) g_hash_table_insert(g_utxo, key, value);
        return true;
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
bool remove_utxo_entry(uint8_t *key) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *key_hex = hash_to_hex(key);
        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);
        sprintf(sql_query, "delete from utxo where hash='%s';\n", key_hex);
        free(key_hex);
        if (!mysql_delete(sql_query)) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to delete UTXO entry.");
            return false;
        }
        if (g_utxo != NULL) g_hash_table_remove(g_utxo, key);
        return true;
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
        general_log(LOG_SCOPE, LOG_DEBUG, "**************************** UTXO *****************************");
        g_hash_table_foreach(g_utxo, print_utxo_entry, NULL);
    }
}

/**
 * Update the block ID in the transaction.
 * @param block_id The block ID to update to.
 * @param txid The transaction ID.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool update_transaction_block_id(unsigned long block_id, uint8_t *txid) {
    char *txid_hex = hash_to_hex(txid);
    char sql_query[1000];
    memset(sql_query, '\0', 1000);
    sprintf(sql_query, "update transaction set block_id=%lu where txid='%s';", block_id, txid_hex);
    free(txid_hex);
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
transaction *get_transaction(uint8_t *txid) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        // Genesis lookups go through the in-memory cache so that test-time
        // mutations of g_genesis_transaction are observed by verify/create.
        if (g_genesis_txid_set && g_genesis_transaction != NULL && memcmp(txid, g_genesis_txid, 32) == 0) {
            return g_genesis_transaction;
        }
        if (g_last_fetched && std::memcmp(txid, g_last_fetched->txid.data(), 32) == 0) {
            return g_last_fetched->tx.get();
        }
        char *txid_hex = hash_to_hex(txid);
        transaction *tx = (transaction *)malloc(sizeof(transaction));

        int temp_sql_query_size = 10000;
        char sql_query[temp_sql_query_size];
        memset(sql_query, '\0', temp_sql_query_size);

        sprintf(sql_query, "select * from transaction where txid='%s';", txid_hex);
        free(txid_hex);
        MYSQL_RES *res = mysql_read(sql_query);
        if (res == NULL) { free(tx); return NULL; }

        // Read transaction.
        MYSQL_ROW row;
        int transaction_auto_id = 0;
        tx->version = 0; tx->tx_in_count = 0; tx->tx_out_count = 0; tx->lock_time = 0;
        bool found = false;
        while ((row = mysql_fetch_row(res))) {
            transaction_auto_id = atoi(row[0]);
            tx->version = atoi(row[2]);
            tx->tx_in_count = atoi(row[3]);
            tx->tx_out_count = atoi(row[4]);
            tx->lock_time = atoi(row[5]);
            found = true;
        }
        mysql_free_result(res);
        if (!found) { free(tx); return NULL; }
        tx->tx_ins = (transaction_input *)malloc(tx->tx_in_count * sizeof(transaction_input));
        memset(tx->tx_ins, 0, tx->tx_in_count * sizeof(transaction_input));
        tx->tx_outs = (transaction_output *)malloc(tx->tx_out_count * sizeof(transaction_output));
        memset(tx->tx_outs, 0, tx->tx_out_count * sizeof(transaction_output));
        memset(sql_query, '\0', temp_sql_query_size);

        // Read transaction outputs.
        sprintf(sql_query, "select * from transaction_output where transaction_id=%d order by id;", transaction_auto_id);
        res = mysql_read(sql_query);
        int output_idx = 0;
        if (res != NULL) {
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
        }
        memset(sql_query, '\0', temp_sql_query_size);

        // Read transaction inputs.
        sprintf(sql_query, "select * from transaction_input where transaction_id=%d order by id;", transaction_auto_id);
        res = mysql_read(sql_query);
        int input_idx = 0;
        int outpoint_input_ids[tx->tx_in_count];
        memset(outpoint_input_ids, 0, tx->tx_in_count * sizeof(int));
        if (res != NULL) {
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
        }
        memset(sql_query, '\0', temp_sql_query_size);

        // Read transaction input's outpoints.
        for (int outpoint_idx = 0; outpoint_idx < tx->tx_in_count; outpoint_idx++) {
            sprintf(sql_query, "select * from transaction_outpoint where transaction_input_id=%d;", outpoint_input_ids[outpoint_idx]);
            res = mysql_read(sql_query);
            if (res == NULL) { memset(sql_query, '\0', temp_sql_query_size); continue; }

            row = mysql_fetch_row(res);
            if (row != NULL) {
                transaction_outpoint *current_outpoint = &tx->tx_ins[outpoint_idx].previous_outpoint;
                current_outpoint->index = atoi(row[2]);
                uint8_t *hash_bin = (uint8_t *)convert_hex_back_to_data_array(row[1]);
                memcpy(current_outpoint->hash, hash_bin, 32);
                free(hash_bin);
            }

            mysql_free_result(res);
            memset(sql_query, '\0', temp_sql_query_size);
        }

        // Cache: optional::emplace replaces any prior entry; the previous
        // unique_ptr's deleter (destroy_transaction) runs automatically.
        cache_entry new_entry;
        std::memcpy(new_entry.txid.data(), txid, 32);
        new_entry.tx.reset(tx);
        g_last_fetched.emplace(std::move(new_entry));
        return tx;
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
        return (transaction *)g_hash_table_lookup(g_global_transaction_table, txid);
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
        uint8_t genesis_txid[32];
        while ((row = mysql_fetch_row(res))) {
            uint8_t *bin = (uint8_t *)convert_hex_back_to_data_array(row[0]);
            memcpy(genesis_txid, bin, 32);
            free(bin);
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
bool does_transaction_exist(uint8_t *txid) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *txid_hex = hash_to_hex(txid);
        char sql_query[1000];
        memset(sql_query, '\0', 1000);
        sprintf(sql_query, "select * from transaction where txid='%s';", txid_hex);
        free(txid_hex);
        MYSQL_RES *res = mysql_read(sql_query);
        if (res == NULL) return false;
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
bool does_utxo_entry_exist(uint8_t *key) {
    if (g_utxo != NULL && g_hash_table_contains(g_utxo, key)) return true;
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *key_hex = hash_to_hex(key);
        char sql_query[1000];
        memset(sql_query, '\0', 1000);
        sprintf(sql_query, "select * from utxo where hash='%s';\n", key_hex);
        free(key_hex);
        MYSQL_RES *res = mysql_read(sql_query);
        if (res == NULL) return false;
        bool result = res->row_count > 0;
        mysql_free_result(res);
        return result;
    }

    return false;
}

/**
 * Destroy the transaction persistence layer
 * by destroying all the tables.
 * @param db_name The name of the MySQL database to use.
 * @return True for success and false otherwise.
 * @author Ing Tian
 */
bool destroy_transaction_persistence(char *db_name) {
    bool res = false;
    g_genesis_transaction = NULL;
    g_genesis_txid_set = false;
    invalidate_get_transaction_cache();
    if (g_utxo != NULL) {
        g_hash_table_destroy(g_utxo);
        g_utxo = NULL;
    }
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        char *sql_query =
            "use %s;\n"
            "drop table if exists utxo;\n"
            "drop table if exists transaction_outpoint;\n"
            "drop table if exists transaction_input;\n"
            "drop table if exists transaction_output;\n"
            "drop table if exists transaction;";
        char filtered_sql_query[1000];
        sprintf(filtered_sql_query, sql_query, db_name);
        res = mysql_delete_table(filtered_sql_query);
        if (!res) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to delete tables.");
        }
    } else if (PERSISTENCE_MODE == PERSISTENCE_RAM) {
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
    uint8_t temp_txid[32];
    while ((row = mysql_fetch_row(res))) {
        uint8_t *bin = (uint8_t *)convert_hex_back_to_data_array(row[0]);
        memcpy(temp_txid, bin, 32);
        free(bin);
    }

    mysql_free_result(res);
    return get_transaction(temp_txid);
}
