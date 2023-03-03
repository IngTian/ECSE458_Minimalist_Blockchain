#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CONSTANTS_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CONSTANTS_H

#include <stdbool.h>

#include "utils/log_utils.h"

// General
#define TOTAL_NUMBER_OF_COINS 40960

// Persistence Definition
#define PERSISTENCE_RAM 0
#define PERSISTENCE_MYSQL 1
#define PERSISTENCE_MODE PERSISTENCE_MYSQL
#define MYSQL_HOST_ADDR "localhost"
#define MYSQL_USERNAME "root"
#define MYSQL_PASSWORD "Admin123/"
#define MYSQL_DB_NAME "test"
#define MYSQL_PORT_NUMBER 3306

// Logging
#define VERBOSE true
#define LOG_LEVEL LOG_INFO
#define LOG_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define LOG_TIME_LENGTH 26

// Cryptography
#define GENESIS_PRIVATE_KEY "FEB634D1D31157FF39BAA3551406BC8D15373AA3D54A6670CDBD28018161969C"

#endif
