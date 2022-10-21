#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H

#include <secp256k1.h>
#include <stdbool.h>
#include <stdlib.h>

secp256k1_context* crypto_context;

typedef unsigned char* private_key;
typedef unsigned char public_key;
typedef secp256k1_pubkey public_key_;
typedef secp256k1_ecdsa_signature crypto_signature;




void initialize_cryptography_system(unsigned int);
void destroy_cryptography_system();
unsigned char *get_a_new_private_key();
secp256k1_pubkey *get_a_new_public_key(char *);
bool verify(secp256k1_pubkey*, unsigned char*, secp256k1_ecdsa_signature*);
secp256k1_ecdsa_signature *sign(private_key private_key, unsigned char *msg_to_sign);
//char* hash_sha256(char*);
#endif
