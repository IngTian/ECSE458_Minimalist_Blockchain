#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H

#include <secp256k1.h>

typedef char *public_key;
typedef char *private_key;

typedef secp256k1_pubkey secp_public_key;
typedef secp256k1_ecdsa_signature secp_signature;


#endif
