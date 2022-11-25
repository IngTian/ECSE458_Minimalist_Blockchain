#include "transaction_persistence.h"

#include "utils/mysql_util.h"

/*
 * -----------------------------------------------------------
 * APIs
 * -----------------------------------------------------------
 */

/**
 * Initialize the persistence layer by creating tables
 * in the database.
 * @return True for success and false otherwise.
 */
bool initialize_transaction_persistence() {
    char *sql_query = "create table transaction\n"
        "(\n"
        "    id           int auto_increment,\n"
        "    txid         char(64)     not null unique,\n"
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
        "    value           bigint         not null,\n"
        "    pk_script_bytes int unsigned   not null,\n"
        "    pk_script       varchar(10000) not null,\n"
        "    transaction_id  int            not null,\n"
        "    primary key (id),\n"
        "    constraint foreign key (transaction_id) references transaction (id)\n"
        ");\n"
        "\n"
        "create table transaction_input\n"
        "(\n"
        "    id               int auto_increment,\n"
        "    script_bytes     int unsigned   not null,\n"
        "    signature_script varchar(10000) not null,\n"
        "    sequence         int unsigned   not null,\n"
        "    transaction_id   int            not null,\n"
        "    primary key (id),\n"
        "    foreign key (transaction_id) references transaction (id)\n"
        ");\n"
        "\n"
        "create table transaction_outpoint\n"
        "(\n"
        "    id                   int auto_increment,\n"
        "    hash                 char(64)     not null,\n"
        "    idx                  int unsigned not null,\n"
        "    transaction_input_id int          not null,\n"
        "    primary key (id),\n"
        "    foreign key (transaction_input_id) references transaction_input (id)\n"
        ");";
    return mysql_create_table(sql_query);
}
bool save_transaction(transaction *);
transaction *get_transaction(char *txid);
bool destroy_transaction_persistence();