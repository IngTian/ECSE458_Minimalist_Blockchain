#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h
#include "../model/block/block.h"
#include "../model/block/block_persistence.h"
#include "../model/transaction/transaction_persistence.h"
#include "pthread.h"
#include "signal.h"
#include "utils/constants.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"
#include "utils/sys_utils.h"

#define LOG_SCOPE "Listener"

void *handle_tcp_connection(void *arg);

int main(int argc, char const *argv[]) {
    // ----------------------------------------
    // Initialize all systems.
    // ----------------------------------------
    initialize_mysql_system(MYSQL_DB_LISTENER);
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    destroy_transaction_system(MYSQL_DB_LISTENER);
    destroy_block_system(MYSQL_DB_LISTENER);
    initialize_transaction_system(true);
    initialize_block_system(true);

    // ----------------------------------------
    // Create and bind the socket; listen on port.
    // ----------------------------------------
    struct addrinfo hints, *serv_addr_info, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;        // Use AF_INET6 to force IPv6.
    hints.ai_socktype = SOCK_STREAM;  // Use socket stream.
    hints.ai_flags = AI_PASSIVE;      // Use my IP address.
    char **general_log_buffer = (char **)malloc((NUMBER_OF_TEST_MODEL * 2 + 2) * sizeof(char *));
    for (int i = 0; i < (NUMBER_OF_TEST_MODEL * 2 + 2); i++) general_log_buffer[i] = (char *)malloc(20);

    int get_addr_res;
    if ((get_addr_res = getaddrinfo(NULL, SERVER_PORT_NO, &hints, &serv_addr_info)) != 0) {
        general_log(LOG_SCOPE, LOG_ERROR, "Failed to get server address info: %s", gai_strerror(get_addr_res));
        exit(1);
    }

    // Loop through all the results and bind to the first we can.
    int server_fd;
    for (p = serv_addr_info; p != NULL; p = p->ai_next) {
        if ((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to create socket.");
            perror("socket");
            continue;
        }
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to bind socket.");
            close(server_fd);
            perror("bind");
            continue;
        }
        if (listen(server_fd, 3) < 0) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed to listen on the port: %s", SERVER_PORT_NO);
            perror("listen");
            exit(EXIT_FAILURE);
        }

        general_log(LOG_SCOPE, LOG_INFO, "Server running at port: %s", SERVER_PORT_NO);
        break;  // if we get here, we must have connected successfully
    }

    // Keep running for listening.
    int client_socket, client_addr_len;
    struct sockaddr_in incoming_client_address;
    while (true) {
        client_addr_len = sizeof(incoming_client_address);
        // Wait for connection.
        if ((client_socket = accept(server_fd, (struct sockaddr *)&incoming_client_address, (socklen_t *)&client_addr_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        general_log(LOG_SCOPE, LOG_INFO, "Handling client %s", inet_ntoa(incoming_client_address.sin_addr));
        pthread_t thread_id;
        int *arg = malloc(sizeof(*arg) * 2);
        *arg = client_socket;
        arg[1] = (int)general_log_buffer;
        pthread_create(&thread_id, NULL, handle_tcp_connection, arg);
    }

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    general_log(LOG_SCOPE, LOG_INFO, "Server shut down.");
    return 0;
}

void *handle_tcp_connection(void *arg) {
    int client_socket = ((int *)arg)[0];
    char msg_buffer[SOCKET_MSG_MAX_SIZE];
    char **general_log_buffer = ((char ***) arg)[1];
    int general_log_counter = 0;
    sprintf(general_log_buffer[general_log_counter++], "Save,Ver");

    for (int i = 0; i < NUMBER_OF_TEST_MODEL; i++) {
        // Get message.
        recv(client_socket, msg_buffer, sizeof(msg_buffer), 0);
        char *received_command = msg_buffer;
        char *data = received_command + COMMAND_LENGTH;

        if (i == 0 || i == NUMBER_OF_TEST_MODEL - 1) {
            general_log(LOG_SCOPE, LOG_INFO, "Received connection %d @(timestamp: %lu): %s", i, get_timestamp(), received_command);
        }

        if (TEST_CREATE_BLOCK) {
            // Receive the block.
            socket_block *received_socket_blk = (socket_block *)malloc(sizeof(socket_block) + ((socket_block *)data)->txns_size);
            memcpy(received_socket_blk, data, sizeof(socket_block) + ((socket_block *)data)->txns_size);
            block *block1 = cast_to_block(received_socket_blk);

            // print block info
            general_log(LOG_SCOPE, LOG_DEBUG, "Block txns count: %d", block1->txn_count);
            general_log(LOG_SCOPE, LOG_DEBUG, "Block header version: %d", block1->header->version);
            char *prev_blk_header_hash = convert_char_hexadecimal(block1->header->prev_block_header_hash, 64);
            general_log(LOG_SCOPE, LOG_DEBUG, "Previous block header hash: %s", prev_blk_header_hash);
            free(prev_blk_header_hash);
            general_log(LOG_SCOPE, LOG_DEBUG, "Block txns count: %d", block1->txn_count);

            received_command = str_trim(received_command);
            if (strcmp(received_command, "genesis block") != 0) {
                // If received block is not genesis, go through normal workflow.
                if (verify_block(block1)) {
                    if (!save_block(block1)) {
                        general_log(LOG_SCOPE, LOG_ERROR, "Saving failed.");
                    }
                } else {
                    general_log(LOG_SCOPE, LOG_ERROR, "Block verification failed.");
                }
            } else {
                if (!save_block(block1)) {
                    general_log(LOG_SCOPE, LOG_ERROR, "Failed to save the genesis block.");
                }
            }
            free(received_command);

            // Free variables.
            free(received_socket_blk);
        } else {
            // Receive the transaction.
            socket_transaction *received_socket_tx = (socket_transaction *)malloc(get_socket_transaction_length((socket_transaction *)data));
            memcpy(received_socket_tx, data, get_socket_transaction_length((socket_transaction *)data));
            transaction *tx = cast_to_transaction(received_socket_tx);

            received_command = str_trim(received_command);
            if (strcmp(received_command, "genesis transaction") != 0) {
                // verification
                unsigned long verification_time = get_timestamp();
                if (verify_transaction(tx)) {
                    // Save to database.
                    verification_time = get_timestamp() - verification_time;
                    unsigned long save_time = get_timestamp();
                    if (!save_transaction(tx)) {
                        general_log(LOG_SCOPE, LOG_ERROR, "Failed to save the transaction.");
                    }
                    save_time = get_timestamp() - save_time;
                    sprintf(general_log_buffer[general_log_counter++], "%lu,%lu", save_time, verification_time);
                } else {
                    general_log(LOG_SCOPE, LOG_ERROR, "Transaction verification failed.");
                }
            } else {
                unsigned long save_timestamp = get_timestamp();

                if (!save_transaction(tx)) {
                    general_log(LOG_SCOPE, LOG_ERROR, "Failed to save the genesis transaction.");
                }
                sprintf(general_log_buffer[general_log_counter++], "%lu,%d", get_timestamp() - save_timestamp, 0);
            }
            // Free variables.
            free(received_command);
            free(received_socket_tx);
        }
        char *send_command = "Done";
        send(client_socket, send_command, strlen(send_command), 0);
    }

    write_to_file(LOGGING_CSV_NAME, LOGGING_MODE, (char **)general_log_buffer, general_log_counter);
    for (int i = 0; i < (NUMBER_OF_TEST_MODEL * 2 + 2); i++) free(general_log_buffer[i]);
    free(general_log_buffer);
    general_log(LOG_SCOPE, LOG_INFO, "%d models done!", NUMBER_OF_TEST_MODEL);
    close(client_socket);
    pthread_exit(NULL);
}
