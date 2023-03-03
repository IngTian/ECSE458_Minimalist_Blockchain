#include "model/transaction/transaction.h"
#include "utils/cryptography.h"

#define LOG_SCOPE "performance test"

int main() {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    initialize_transaction_system();
    char *private_key = get_genesis_transaction_private_key();
    return 0;
}
