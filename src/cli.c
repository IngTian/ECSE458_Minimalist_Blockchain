#include "cli.h"

#include <assert.h>
#include <glib.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>

#include "random.h"
#include "utils/cryptography.h"
#include "model/transaction/transaction.h"

int main(int argc, char *argv[]) {
    initialize_transaction_system();

    struct Transaction transaction1;
    transaction1.version=1;
    transaction1.tx_in_count=100;
    transaction1.tx_out_count=50;

    transaction_outpoint outpoint1;
    outpoint1.hash=111;
    outpoint1.index=1;

    transaction_outpoint outpoint2;
    outpoint1.hash=222;
    outpoint1.index=2;

    return 0;
}
