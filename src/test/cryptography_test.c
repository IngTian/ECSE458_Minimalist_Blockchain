#include "cli.h"

#include <assert.h>
#include <glib.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>

#include "random.h"
#include "utils/cryptography.h"

int main(int argc, char *argv[]) {

    //Set the variables
    unsigned char msgHash[32]={
        0x31, 0x5F, 0x5B, 0xDB, 0x76, 0xD0, 0x78, 0xC4, 0x3B, 0x8A, 0xC0, 0x06, 0x4E, 0x4A, 0x01, 0x64,
        0x61, 0x2B, 0x1F, 0xCE, 0x77, 0xC8, 0x69, 0x34, 0x5B, 0xFC, 0x94, 0x99, 0x58, 0x94, 0xE1, 0xD3,
    };
    private_key privateKey;
    public_key_* publicKey;
    unsigned char compactedPublicKey[33];
    crypto_signature* signature;
    unsigned char compactedSignature[64];


    //Initialize the system, create a context
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

    //Generate a random private key
    privateKey=get_a_new_private_key();
    printf("Private Key:\n");
    print_hex(privateKey, sizeof(privateKey));

    //Use private key to generate public key
    publicKey=get_a_new_public_key(privateKey);
    size_t len=sizeof(compactedPublicKey);
    secp256k1_ec_pubkey_serialize(crypto_context, compactedPublicKey, &len, publicKey, SECP256K1_EC_COMPRESSED);
    printf("Public Key: \n");
    print_hex(compactedPublicKey,sizeof (compactedPublicKey));

    //Use private key and the message to create a signature
    signature=sign(privateKey,msgHash);
    secp256k1_ecdsa_signature_serialize_compact(crypto_context,compactedSignature,signature);
    printf("Signature: \n");
    print_hex(compactedSignature,sizeof (compactedSignature));

    //Verify if the public key matches the signature
    bool is_signature_valid= verify(publicKey,msgHash,signature);
    printf("Is the signature valid? %s\n", is_signature_valid ? "true" : "false");

    //Destroy the context object
    destroy_cryptography_system();

    return 0;
}
