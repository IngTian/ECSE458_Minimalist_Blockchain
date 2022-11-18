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

void initialize_mysql_system(mysql_config);
bool mysql_create_database(char *sql_query);
bool mysql_destroy_database(char *sql_query);
bool mysql_create(char *sql_query);
MYSQL_RES *mysql_read(char *sql_query);
bool mysql_update(char *sql_query);
bool mysql_delete(char *sql_query);
void destroy_mysql_system();

#endif