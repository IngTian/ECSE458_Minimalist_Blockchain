#include "mysql_util.h"

#include <stdio.h>
#include <stdlib.h>

#define LOG_SCOPE "mysql_util"

MYSQL *g_mysql_client;

void finish_with_error() {
    //fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
    general_log(LOG_SCOPE, LOG_ERROR, "my_sql query error: %s", mysql_error(g_mysql_client));
    mysql_close(g_mysql_client);
    exit(1);
}

void initialize_mysql_system(mysql_config) {
    mysql_init(g_mysql_client);

    if (g_mysql_client == NULL) {
        //fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
        general_log(LOG_SCOPE, LOG_ERROR, "my_sql initialize error: %s", mysql_error(g_mysql_client));
        exit(1);
    }

    if (mysql_real_connect(g_mysql_client, mysql_config->host_addr, mysql_config->username, mysql_config->password, mysql_config->db,
                           mysql_config->port_number, mysql_config->unix_socket, mysql_config->client_flag) == NULL) {
        //fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
        general_log(LOG_SCOPE, LOG_ERROR, "my_sql real connect error: %s", mysql_error(g_mysql_client));
        mysql_close(g_mysql_client);
        exit(1);
    }

    // if (mysql_query(g_mysql_client, "CREATE DATABASE testdb")) {
    //     fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
    //     mysql_close(g_mysql_client);
    //     exit(1);
    // }
}

bool mysql_create_database(char *sql_query) {
    if (mysql_query(g_mysql_client, sql_query)) {
        finish_with_error(g_mysql_client);
    }
    return True;
}

bool mysql_destroy_database(char *sql_query) {
    if (mysql_query(g_mysql_client, sql_query)) {
        finish_with_error(g_mysql_client);
    }
    return True;
}

bool mysql_create(char *sql_query) {
    if (mysql_query(g_mysql_client, sql_query)) {
        finish_with_error(g_mysql_client);
    }
    return True;
}

MYSQL_RES mysql_read(char *sql_query) {
    if (mysql_query(g_mysql_client, sql_query)) {
        finish_with_error(g_mysql_client);
    }
    MYSQL_RES *result = mysql_store_result(g_mysql_client);
    return result;
}

bool mysql_update(char *sql_query) {
    if (mysql_query(g_mysql_client, sql_query)) {
        finish_with_error(g_mysql_client);
    }
    return True;
}
bool mysql_delete(char *sql_query) {
    if (mysql_query(g_mysql_client, sql_query)) {
        finish_with_error(g_mysql_client);
    }
    return True;
}

// int get_previous_insert_id(MYSQL *mysql_database) {
//     int id = mysql_insert_id(mysql_database);
//     printf("The last inserted row id is: %d\n", id);

//     return id;
// }

void mysql_delete_result(MYSQL_RES *result) { free(result); }

void destroy_mysql_system() {
    mysql_close(g_mysql_client);
    free(g_mysql_client);
}

int main(int argc, char **argv) {}