#include "cli.h"

#include <assert.h>
#include <glib.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>

#include "random.h"

secp256k1_context* create_context(unsigned int flag){
    secp256k1_context* ctx= (secp256k1_context *)malloc(sizeof (ctx));
    ctx=secp256k1_context_create(flag);
    unsigned char randomize[32];
    fill_random(randomize, sizeof(randomize));
    secp256k1_context_randomize(ctx, randomize);
    return ctx;
}

void generate_public_key(const secp256k1_context *ctx, secp256k1_pubkey *pubkey, const unsigned char *seckey, unsigned char *output){
    int ret_val=secp256k1_ec_pubkey_create(ctx,pubkey,seckey);
    size_t len = sizeof(output);
    printf("%d\n",len);
    secp256k1_ec_pubkey_serialize(ctx, output, &len, &pubkey, SECP256K1_EC_COMPRESSED);
    printf("he\n");

}

void sign(const secp256k1_context* ctx, secp256k1_ecdsa_signature sig, unsigned char msg_hash[32], const unsigned char *seckey, unsigned char serialized_signature[64]){
    int return_val = secp256k1_ecdsa_sign(ctx, &sig, msg_hash, seckey, NULL, NULL);
    /* Serialize the signature in a compact form. Should always return 1
     * according to the documentation in secp256k1.h. */
    return_val = secp256k1_ecdsa_signature_serialize_compact(ctx, serialized_signature, &sig);
}

bool verify(const secp256k1_context* ctx, secp256k1_ecdsa_signature sig, unsigned char msg_hash[32], secp256k1_pubkey pubkey){
    int is_signature_valid = secp256k1_ecdsa_verify(ctx, &sig, msg_hash, &pubkey);
    if(is_signature_valid==1){
        return true;
    } else{
        return false;
    }
}


int main(int argc, char *argv[]) {
    GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(hash, "Jazzy", "Cheese");
    g_hash_table_insert(hash, "Mr Darcy", "Treats");

    printf("There are %d keys in the hash table\n", g_hash_table_size(hash));

    printf("Jazzy likes %s\n", g_hash_table_lookup(hash, "Jazzy"));

    unsigned char msg_hash[32] = {
        0x31, 0x5F, 0x5B, 0xDB, 0x76, 0xD0, 0x78, 0xC4,
        0x3B, 0x8A, 0xC0, 0x06, 0x4E, 0x4A, 0x01, 0x64,
        0x61, 0x2B, 0x1F, 0xCE, 0x77, 0xC8, 0x69, 0x34,
        0x5B, 0xFC, 0x94, 0xC7, 0x58, 0x94, 0xED, 0xD3,
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




    secp256k1_context* ctx = create_context(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    /*** Key Generation ***/

    /* If the secret key is zero or out of range (bigger than secp256k1's
     * order), we try to sample a new key. Note that the probability of this
     * happening is negligible. */


    generate_public_key(ctx,&pubkey,seckey,compressed_pubkey);


    /*** Signing ***/

    /* Generate an ECDSA signature `noncefp` and `ndata` allows you to pass a
     * custom nonce function, passing `NULL` will use the RFC-6979 safe default.
     * Signing with a valid context, verified secret key
     * and the default nonce function should never fail. */
    sign(ctx,sig,msg_hash,seckey,serialized_signature);


    /*** Verification ***/

    /* Verify a signature. This will return 1 if it's valid and 0 if it's not. */
    is_signature_valid = verify(ctx, sig, msg_hash, pubkey);

    printf("Is the signature valid? %s\n", is_signature_valid ? "true" : "false");
    printf("Secret Key: ");
    print_hex(seckey, sizeof(seckey));
    printf("Public Key: ");
    print_hex(compressed_pubkey, sizeof(compressed_pubkey));
    printf("Signature: ");
    print_hex(serialized_signature, sizeof(serialized_signature));


    /* This will clear everything from the context and free the memory */
    secp256k1_context_destroy(ctx);

    /* It's best practice to try to clear secrets from memory after using them.
     * This is done because some bugs can allow an attacker to leak memory, for
     * example through "out of bounds" array access (see Heartbleed), Or the OS
     * swapping them to disk. Hence, we overwrite the secret key buffer with zeros.
     *
     * TODO: Prevent these writes from being optimized out, as any good compiler
     * will remove any writes that aren't used. */
    memset(seckey, 0, sizeof(seckey));

    return 0;
}
