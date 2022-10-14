#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H

#include <secp256k1.h>
#include <stdbool.h>

typedef char *public_key;
typedef char *private_key;


typedef struct cryptography_object {

} crypto_object;

secp256k1_context* create_context(unsigned int flag);
void generate_public_key(const secp256k1_context *ctx, secp256k1_pubkey *pubkey, const unsigned char *seckey);
void sign(const secp256k1_context* ctx, secp256k1_ecdsa_signature* sig, unsigned char *msg_hash, const unsigned char *seckey, unsigned char *serialized_signature);
bool verify(const secp256k1_context* ctx, secp256k1_ecdsa_signature* sig, unsigned char* msg_hash, secp256k1_pubkey *pubkey);
#endif
