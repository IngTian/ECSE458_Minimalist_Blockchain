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
    //Init the transaction system
    initialize_transaction_system();
    //Create a coin pool
    register_coin_pool();

    //Create a transaction link to the register coin pool
    transaction* t1=create_new_transaction();
    t1->version=223;
    transaction_outpoint outpoint1;
    outpoint1.hash=POOL_TXID;
    outpoint1.index=0;

    transaction_receive_coin(t1,&outpoint1,NULL);


    return 0;
}
