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

    register_coin_pool(99999999,114514);

    get_all_transaction();




    transaction t1;
    t1.version=1;
    t1.tx_in_count=1;
    t1.tx_out_count=0;
    transaction_outpoint outpoint1;
    outpoint1.hash=114514;
    outpoint1.index=1;

    transaction_input **tx_ins= malloc(sizeof(transaction_input));
    transaction_input *input=malloc(sizeof(transaction_input));;

    input->outpoint=&outpoint1;

    tx_ins[0]=input;

    t1.tx_ins=tx_ins;


    register_transaction_in_system(&t1,true);





    return 0;
}
