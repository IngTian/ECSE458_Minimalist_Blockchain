#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_UTILS_MYSQL_UTIL_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_UTILS_MYSQL_UTIL_H

#include <mysql.h>

typedef struct MySQLConfig {
    char host_addr[50];
    char username[50];
    char password[50];
    char *db;
    unsigned int port_number;
    char *unix_socket;
    unsigned long client_flag;
} mysql_config;

void initialize_mysql_system();
bool mysql_create_database(char *sql_query);
bool mysql_create_table(char *sql_query);
bool mysql_delete_table(char *sql_query);
bool mysql_insert(char *sql_query);
MYSQL_RES *mysql_read(char *sql_query);
bool mysql_update(char *sql_query);
bool mysql_delete(char *sql_query);
unsigned long mysql_get_last_updated_id();
void destroy_mysql_system();

#endif