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



    char *buf=hash_sha256("caonimabi");
    print_hex(buf,32);


    return 0;
}
