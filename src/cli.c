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

    register_coin_pool();


    //Create a transaction link to the register coin pool
    transaction* t1=create_new_transaction();
    t1->version=223;
    transaction_outpoint outpoint1;
    outpoint1.hash=POOL_TXID;
    outpoint1.index=0;

    transaction_receive_coin(t1,&outpoint1);
    int t2receiveIndex=transaction_send_coin(t1,5000);
    int t3receiveIndex=transaction_send_coin(t1,7000);

    transaction_outpoint outpoint2;
    outpoint2.hash= get_transaction_txid(t1);
    outpoint2.index=t2receiveIndex;

    transaction_outpoint outpoint3;
    outpoint3.hash= get_transaction_txid(t1);
    outpoint3.index=t3receiveIndex;

    register_transaction_in_system(t1,true);

    transaction* t2=create_new_transaction();
    transaction* t3=create_new_transaction();

    transaction_receive_coin(t2,&outpoint2);
    transaction_receive_coin(t3,&outpoint3);

    register_transaction_in_system(t2,true);
    register_transaction_in_system(t3,true);

    get_all_transaction();
    print_UTXO();

    return 0;
}
