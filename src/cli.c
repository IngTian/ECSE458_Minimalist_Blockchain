#include "cli.h"

#include <assert.h>
#include <glib.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>

#include "model/transaction/transaction.h"
#include "random.h"
#include "utils/cryptography.h"

int main(int argc, char *argv[]) {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_transaction = initialize_transaction_system();
    print_all_transactions();
    print_utxo();

    transaction *t = create_an_empty_transaction();

    char *genesis_transaction_id = get_transaction_txid(genesis_transaction);
    transaction_input input = {
        .previous_outpoint = {.index = 0}, .signature_script = (char *)malloc(64), .sequence = 1, .script_bytes = 64};
    memcpy(input.previous_outpoint.hash, genesis_transaction_id, 64);
    free(genesis_transaction_id);

    char *msg_hash = hash_transaction_output(&genesis_transaction->tx_outs[0]);
    char *private_key = get_genesis_transaction_private_key();
    secp256k1_ecdsa_signature *signature = sign((unsigned char *)private_key, (unsigned char *)msg_hash);
    memcpy(input.signature_script, signature->data, 64);
    free(signature);
    free(msg_hash);
    if (!append_new_transaction_input(t, input)) exit(1);

    char *pk = (char *)malloc(64);
    memset(pk, 0, 64);
    transaction_output output = {.value = 4096, .pk_script = pk, .pk_script_bytes = 64};
    if (!append_new_transaction_output(t, output)) exit(2);

    print_all_transactions();
    print_utxo();

    return 0;
}
