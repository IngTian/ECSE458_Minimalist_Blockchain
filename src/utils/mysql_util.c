#include "mysql_util.h"

#include <stdlib.h>

#include "constants.h"
#include "log_utils.h"

#define LOG_SCOPE "mysql_util"

MYSQL *g_mysql_connection;

/*
 * -----------------------------------------------------------
 * Helper methods.
 * -----------------------------------------------------------
 */
void free_mysql_connection_result() {
    while (mysql_more_results(g_mysql_connection)) {
        MYSQL_RES *result = mysql_store_result(g_mysql_connection);
        mysql_free_result(result);
        mysql_next_result(g_mysql_connection);
    }
}

/*
 * -----------------------------------------------------------
 * APIs
 * -----------------------------------------------------------
 */

/**
 * Initialize the MySQL system.
 * @param config The config passed on to initialize the MySQL.
 * @param db_name The database name.
 * @author Luke E
 */
void initialize_mysql_system(char *db_name) {
    if (PERSISTENCE_MODE == PERSISTENCE_MYSQL) {
        general_log(LOG_SCOPE, LOG_INFO, "MySQL client version detected: %s", mysql_get_client_info());

        g_mysql_connection = mysql_init(NULL);

        if (g_mysql_connection == NULL) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to initialize the MYSQL object: %s", mysql_error(g_mysql_connection));
            exit(1);
        }

        mysql_config config = {.host_addr = MYSQL_HOST_ADDR,
                               .username = MYSQL_USERNAME,
                               .password = MYSQL_PASSWORD,
                               .db = db_name,
                               .port_number = MYSQL_PORT_NUMBER,
                               .client_flag = CLIENT_MULTI_STATEMENTS};

        if (mysql_real_connect(g_mysql_connection,
                               config.host_addr,
                               config.username,
                               config.password,
                               config.db,
                               config.port_number,
                               config.unix_socket,
                               config.client_flag) == NULL) {
            general_log(LOG_SCOPE, LOG_ERROR, "Error in connecting to MySQL: %s", mysql_error(g_mysql_connection));
            mysql_close(g_mysql_connection);
            exit(1);
        }
    }
}

/**
 * Create a database.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool mysql_create_database(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to create database (%s) with SQL: %s", mysql_error(g_mysql_connection), sql_query);
        return false;
    }
    return true;
}

/**
 * Create tables.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool mysql_create_table(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to create tables (%s) with SQL: %s", mysql_error(g_mysql_connection), sql_query);
        return false;
    }
    return true;
}

/**
 * Delete a table from the database.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool mysql_delete_table(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to delete tables with SQL: %s", sql_query);
        return false;
    }
    return true;
}

/**
 * Insert the record into the database.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool mysql_insert(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to insert into the database (%s) with SQL: %s", mysql_error(g_mysql_connection), sql_query);
        return false;
    }
    return true;
}

/**
 * Read records from the SQL database.
 * @param sql_query A SQL query.
 * @return The SQL result.
 * @author Luke E
 */
MYSQL_RES *mysql_read(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to read from the database (%s) with SQL: %s", mysql_error(g_mysql_connection), sql_query);
        return NULL;
    }
    return mysql_store_result(g_mysql_connection);
}

/**
 * Update entries in the table.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool mysql_update(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to update data (%s) with SQL: %s", mysql_error(g_mysql_connection), sql_query);
        return false;
    }
    return true;
}

/**
 * Delete entries in the table.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @auhtor Luke E
 */
bool mysql_delete(char *sql_query) {
    free_mysql_connection_result();
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to delete entries (%s) with SQL: %s", mysql_error(g_mysql_connection), sql_query);
        return false;
    }
    return true;
}

/**
 * Get the last updated ID in the database.
 * @return The last updated ID.
 * @author Luke E
 */
unsigned long mysql_get_last_updated_id() { return mysql_insert_id(g_mysql_connection); }

/**
 * Destroy the MySQL Util system.
 * @auhtor Luke E
 */
void destroy_mysql_system() {
    mysql_close(g_mysql_connection);
    free(g_mysql_connection);
}