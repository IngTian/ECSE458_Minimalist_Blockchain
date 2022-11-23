#include "mysql_util.h"

#include <stdlib.h>

#include "log_utils.h"

#define LOG_SCOPE "mysql_util"

MYSQL *g_mysql_connection;

/*
 * -----------------------------------------------------------
 * APIs
 * -----------------------------------------------------------
 */

/**
 * Initialize the MySQL system.
 * @param config The config passed on to initialize the MySQL.
 * @author Luke E
 */
void initialize_mysql_system(mysql_config config) {
    general_log(LOG_SCOPE, LOG_INFO, "MySQL client version detected: %s\n", mysql_get_client_info());

    g_mysql_connection = mysql_init(NULL);

    if (g_mysql_connection == NULL) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to initialize the MYSQL object: %s", mysql_error(g_mysql_connection));
        exit(1);
    }

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

/**
 * Create a database.
 * @param sql_query A SQL query.
 * @return True for success and false otherwise.
 * @author Luke E
 */
bool mysql_create_database(char *sql_query) {
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_DEBUG, "Failed to create database with SQL: %s", sql_query);
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
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_DEBUG, "Failed to delete tables with SQL: %s", sql_query);
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
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_DEBUG, "Failed to insert into the database with SQL: %s", sql_query);
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
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_DEBUG, "Failed to read from the database with SQL: %s", sql_query);
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
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_DEBUG, "Failed to update data with SQL: %s", sql_query);
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
    if (mysql_query(g_mysql_connection, sql_query)) {
        general_log(LOG_SCOPE, LOG_DEBUG, "Failed to delete entries with SQL: %s", sql_query);
        return false;
    }
    return true;
}

/**
 * Destroy the MySQL Util system.
 * @auhtor Luke E
 */
void destroy_mysql_system() {
    mysql_close(g_mysql_connection);
    free(g_mysql_connection);
}