#include "mysql_util.h"
#include <stdio.h>
#include <stdlib.h>

MYSQL *g_mysql_client;


bool initialize_mysql_system() {
    g_mysql_client = mysql_init(NULL);

    if (g_mysql_client == NULL) {
        fprintf(stderr, "%s\n", mysql_error(g_mysql_client));
        exit(1);
    }

    if (mysql_real_connect(g_mysql_client, "localhost", "root", "root_passwd", NULL, 0, NULL, 0) == NULL) {
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

int get_previous_insert_id(MYSQL *mysql_database) {
    int id = mysql_insert_id(mysql_database);
    printf("The last inserted row id is: %d\n", id);

    return id;
}

void mysql_retrieve_data(MYSQL *mysql_database) {
    MYSQL_RES *result = mysql_store_result(mysql_database);

    if (result == NULL) {
        finish_with_error(mysql_database);
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

    mysql_free_result(result);
}


void destroy_mysql_connection(MYSQL *mysql_database) {
    mysql_close(mysql_database);
    exit(0);
}


void finish_with_error(MYSQL *mysql_database) {
    fprintf(stderr, "%s\n", mysql_error(mysql_database));
    mysql_close(mysql_database);
    exit(1);
}


int main(int argc, char **argv) {
    initialize_mysql_system(g_mysql_client);
    execute_mysql_query(g_mysql_client, "CREATE TABLE cars(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255), price INT)");
    execute_mysql_query(g_mysql_client, "INSERT INTO cars VALUES(1,'Audi',52642)");
    destroy_mysql_connection(g_mysql_client);
}