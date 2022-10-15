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
    GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(hash, "Jazzy", "Cheese");
    g_hash_table_insert(hash, "Mr Darcy", "Treats");

    printf("There are %d keys in the hash table\n", g_hash_table_size(hash));

    printf("Jazzy likes %s\n", g_hash_table_lookup(hash, "Jazzy"));

    initialize_transaction_system();

    return 0;
}
