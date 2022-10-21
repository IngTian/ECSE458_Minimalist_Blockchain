#include "cryptography.h"

#include <assert.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cryptography.h"
#include "random.h"

void initialize_cryptography_system(unsigned int flag) { crypto_context = 
    secp256k1_context_create(flag); 
}

void destroy_cryptography_system() { secp256k1_context_destroy(crypto_context);}


secp256k1_ecdsa_signature *sign(unsigned char *private_key, unsigned char *msg_to_sign) {
    secp256k1_ecdsa_signature* signature=(secp256k1_ecdsa_signature *)malloc
        (sizeof(secp256k1_ecdsa_signature));
    secp256k1_ecdsa_sign(crypto_context, signature, msg_to_sign, private_key, NULL, NULL);
    return signature;

}

unsigned char* compact_signature(secp256k1_ecdsa_signature *signature){
    unsigned char* compacted_signature= (char *)malloc(sizeof (char)*64);
    secp256k1_ecdsa_signature_serialize_compact(crypto_context, compacted_signature, signature);
    return compacted_signature;
}

bool verify(secp256k1_pubkey *public_key, unsigned char *msg_hash, secp256k1_ecdsa_signature *signature) {
    int is_signature_valid = secp256k1_ecdsa_verify(crypto_context, signature, msg_hash, public_key);
    if (is_signature_valid == 1) {
        return true;
    } else {
        return false;
    }
}

unsigned char* get_a_new_private_key() {
    unsigned char* private_key= (char *)malloc(sizeof (char)*64);
    while (1) {
        if (!fill_random(private_key,32)) {
            printf("Failed to generate randomness\n");
            return NULL;
        }
        if (secp256k1_ec_seckey_verify(crypto_context, private_key)) {
            return private_key;
        }
    }
}

secp256k1_pubkey *get_a_new_public_key(char *private_key) {
    secp256k1_pubkey *ret_val = (secp256k1_pubkey *)malloc(sizeof(secp256k1_pubkey));
    secp256k1_ec_pubkey_create(crypto_context, ret_val, private_key);
    return ret_val;
}

// char* hash_sha256(char*){

// }