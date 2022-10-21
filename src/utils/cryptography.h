#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H

#include <secp256k1.h>
#include <stdbool.h>
#include <stdlib.h>

/**    sha256 utils import   **/
#include <stddef.h>

/**    sha256 macro   **/
#define SHA256_BLOCK_SIZE 32

/**    sha256 data types   **/
typedef unsigned char BYTE;  // 8-bit byte
typedef unsigned int WORD;   // 32-bit word, change to "long" for 16-bit machines

typedef struct {
    BYTE data[64];
    WORD datalen;
    unsigned long long bitlen;
    WORD state[8];
} SHA256_CTX;

/**    sha256 functions   **/
char *hash_struct(void *, unsigned int);
char *hash_sha256(char *);
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);

/**    public/private key   **/

secp256k1_context *crypto_context;

typedef unsigned char *private_key;
typedef unsigned char public_key;
typedef secp256k1_pubkey public_key_;
typedef secp256k1_ecdsa_signature crypto_signature;

void initialize_cryptography_system(unsigned int);
void destroy_cryptography_system();
unsigned char *get_a_new_private_key();
secp256k1_pubkey *get_a_new_public_key(char *);
bool verify(secp256k1_pubkey *, unsigned char *, secp256k1_ecdsa_signature *);
secp256k1_ecdsa_signature *sign(private_key private_key, unsigned char *msg_to_sign);
// char* hash_sha256(char*);
#endif
