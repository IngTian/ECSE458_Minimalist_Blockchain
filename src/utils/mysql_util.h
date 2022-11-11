#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_UTILS_MYSQL_UTIL_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_SRC_UTILS_MYSQL_UTIL_H

#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>


void initialize_mysql_system(MYSQL *mysql_database);
int get_previous_insert_id(MYSQL *mysql_database);
void mysql_retrieve_data(MYSQL *mysql_database);
void destroy_mysql_connection(MYSQL *mysql_database);