#include "mysql_util.h"

#include <stdio.h>
#include <stdlib.h>

MYSQL *g_mysql_client;

void initialize_mysql_system(mysql_config) {
    mysql_init(g_mysql_client);

    if (g_mysql_client == NULL) {
        fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
        exit(1);
    }

    if (mysql_real_connect(g_mysql_client, mysql_config->host_addr, mysql_config->username, mysql_config->password, mysql_config->db,
                           mysql_config->port_number, mysql_config->unix_socket, mysql_config->client_flag) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
        mysql_close(g_mysql_client);
        exit(1);
    }

    if (mysql_query(g_mysql_client, "CREATE DATABASE testdb")) {
        fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
        mysql_close(g_mysql_client);
        exit(1);
    }
}

bool mysql_create_database(char *sql_query) {
    if (mysql_query(g_mysql_client, char *sql_query)) {
        finish_with_error(con);
    }
    return 1;
}

bool mysql_destroy_database(char *sql_query) {
    if (mysql_query(g_mysql_client, char *sql_query)) {
        finish_with_error(con);
    }
    return 1;
}

bool mysql_create(char *sql_query) {
    if (mysql_query(g_mysql_client, char *sql_query)) {
        finish_with_error(con);
    }
    return 1;
}

bool mysql_read(char *sql_query){
    if (mysql_query(g_mysql_client, char *sql_query)) {
        finish_with_error(con);
    }
    return 1;
}


bool mysql_update(char *sql_query) {
    if (mysql_query(g_mysql_client, char *sql_query)) {
        finish_with_error(con);
    }
    return 1;
}
bool mysql_delete(char *sql_query) {
    if (mysql_query(g_mysql_client, char *sql_query)) {
        finish_with_error(con);
    }
    return 1;
}

// int get_previous_insert_id(MYSQL *mysql_database) {
//     int id = mysql_insert_id(mysql_database);
//     printf("The last inserted row id is: %d\n", id);

//     return id;
// }

MYSQL_RES *mysql_retrieve_data() {
    MYSQL_RES *result = mysql_store_result(g_mysql_client);

    if (result == NULL) {
        finish_with_error(g_mysql_client);
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    MYSQL_FIELD *field;

    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < num_fields; i++) {
            if (i == 0) {
                while (field = mysql_fetch_field(result)) {
                    printf("%s ", field->name);
                }
                printf("\n");
            }
            printf("%s ", row[i] ? row[i] : "NULL");
        }

        printf("\n");
    }

    return result;
}

void destroy_mysql_system() {
    mysql_close(g_mysql_client);
    mysql_free_result(result);
    free(g_mysql_client);
    exit(0);
}

void finish_with_error() {
    fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
    mysql_close(g_mysql_client);
    exit(1);
}

int main(int argc, char **argv) {

}