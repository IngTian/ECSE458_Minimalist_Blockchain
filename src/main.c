#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "model/transaction/transaction.h"
#include "model/transaction/transaction_persistence.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "utils/log_utils.h"
#include "utils/mysql_util.h"

#define LOG_SCOPE "benchmark"

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void bench_1in_1out_chain(int n) {
    initialize_mysql_system("test");
    destroy_transaction_persistence("test");
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_t = initialize_transaction_system(false);

    uint8_t *prev_txid = get_transaction_txid(genesis_t);
    char *prev_priv = get_genesis_transaction_private_key();

    double t_start = now_seconds();
    for (int i = 0; i < n; i++) {
        uint8_t *new_txid = NULL;
        char *new_priv = NULL;
        transaction *t = create_a_new_single_in_single_out_transaction(
            prev_txid, prev_priv, 0, TOTAL_NUMBER_OF_COINS, &new_txid, &new_priv);
        if (t == NULL || new_txid == NULL) {
            general_log(LOG_SCOPE, LOG_ERROR, "Failed at iteration %d", i);
            break;
        }
        free(prev_txid);
        prev_txid = new_txid;
        prev_priv = new_priv;
    }
    double t_end = now_seconds();
    free(prev_txid);

    double elapsed = t_end - t_start;
    double tps = (double)n / elapsed;
    printf("[1-in-1-out] N=%d  elapsed=%.4fs  throughput=%.2f TX/sec\n", n, elapsed, tps);

    destroy_transaction_system("test");
    destroy_cryptography_system();
    destroy_mysql_system();
}

int main(int argc, char *argv[]) {
    int sizes[] = {100, 500, 1000};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    if (argc > 1) {
        sizes[0] = atoi(argv[1]);
        num_sizes = 1;
    }

    printf("=== Blockchain Throughput Benchmark ===\n");
    printf("Persistence: MYSQL / INNODB (disk)\n");
    printf("Methodology: chain of 1-in-1-out transactions; time from first to last create+finalize\n\n");

    for (int i = 0; i < num_sizes; i++) {
        bench_1in_1out_chain(sizes[i]);
    }

    return 0;
}
