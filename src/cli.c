#include "cli.h"

#include <assert.h>
#include <glib.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>

#include "random.h"
#include "utils/cryptography.h"


int main(int argc, char *argv[]) {
    GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(hash, "Jazzy", "Cheese");
    g_hash_table_insert(hash, "Mr Darcy", "Treats");

    printf("There are %d keys in the hash table\n", g_hash_table_size(hash));

    printf("Jazzy likes %s\n", g_hash_table_lookup(hash, "Jazzy"));

    //public/private key test

    unsigned char msg_hash[32] = {
        0x31, 0x5F, 0x5B, 0xDB, 0x76, 0xD0, 0x78, 0xC4,
        0x3B, 0x8A, 0xC0, 0x06, 0x4E, 0x4A, 0x01, 0x64,
        0x61, 0x2B, 0x1F, 0xCE, 0x77, 0xC8, 0x69, 0x34,
        0x5B, 0xFC, 0x94, 0x99, 0x58, 0x94, 0xE1, 0xD3,
    };
    unsigned char seckey[32];
    unsigned char randomize[32];
    unsigned char compressed_pubkey[33];
    unsigned char serialized_signature[64];
    size_t len;
    int is_signature_valid;
    int return_val;
    secp256k1_pubkey pubkey;
    secp256k1_ecdsa_signature sig;
    /* The specification in secp256k1.h states that `secp256k1_ec_pubkey_create` needs
     * a context object initialized for signing and `secp256k1_ecdsa_verify` needs
     * a context initialized for verification, which is why we create a context
     * for both signing and verification with the SECP256K1_CONTEXT_SIGN and
     * SECP256K1_CONTEXT_VERIFY flags. */


    /** Create a context**/

    secp256k1_context* ctx = create_context(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

    /*** Key Generation ***/

    /* Fill random to seckey*/
    int i=0;
    while (1) {
        if (!fill_random(seckey, sizeof(seckey))) {
            printf("Failed to generate randomness\n");
            return 1;
        }
        if (secp256k1_ec_seckey_verify(ctx, seckey)) {
            break;
        }
    }

    /* Use secret key to generate public key*/
    generate_public_key(ctx, &pubkey, seckey);

    /* Serialize the pubkey in a compressed form(33 bytes). */
    len = sizeof(compressed_pubkey);
    return_val = secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &len, &pubkey, SECP256K1_EC_COMPRESSED);

    /*** Signing ***/

    /* Generate a EDCSA signature with a message hash and secret key */
    /* Will generate serialized version of signature as well */
    sign(ctx, &sig, msg_hash, seckey,serialized_signature);



    /*** Verification ***/

    /* Verify a signature. This will return 1 if it's valid and 0 if it's not. */
    is_signature_valid = verify(ctx, &sig, msg_hash, &pubkey);





    printf("Is the signature valid? %s\n", is_signature_valid ? "true" : "false");
    printf("Secret Key: ");
    print_hex(seckey, sizeof(seckey));
    printf("Public Key: ");
    print_hex(compressed_pubkey, sizeof(compressed_pubkey));
    printf("Signature: ");
    print_hex(serialized_signature, sizeof(serialized_signature));


    /* This will clear everything from the context and free the memory */
    secp256k1_context_destroy(ctx);

    memset(seckey, 0, sizeof(seckey));

    return 0;
}
