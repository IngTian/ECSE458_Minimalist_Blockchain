#include "cryptography.h"

#include <assert.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "random.h"


secp256k1_context* create_context(unsigned int flag){
    secp256k1_context* ctx;
    ctx=secp256k1_context_create(flag);
    //    fill_random(randomize, sizeof(randomize));
    //    secp256k1_context_randomize(ctx, randomize);
    return ctx;
}

void generate_public_key(const secp256k1_context *ctx, secp256k1_pubkey *pubkey, const unsigned char *seckey){
    fill_random(seckey,sizeof (seckey));
    secp256k1_ec_pubkey_create(ctx,pubkey,seckey);
    //    size_t len = sizeof(output);
    //    printf("%d\n",len);
    //    secp256k1_ec_pubkey_serialize(ctx, output, &len, &pubkey, SECP256K1_EC_COMPRESSED);


}

void sign(const secp256k1_context* ctx, secp256k1_ecdsa_signature* sig, unsigned char *msg_hash, const unsigned char *seckey, unsigned char *serialized_signature){
    secp256k1_ecdsa_sign(ctx, sig, msg_hash, seckey, NULL, NULL);
    secp256k1_ecdsa_signature_serialize_compact(ctx, serialized_signature, sig);
}

bool verify(const secp256k1_context* ctx, secp256k1_ecdsa_signature* sig, unsigned char* msg_hash, secp256k1_pubkey *pubkey){
    int is_signature_valid = secp256k1_ecdsa_verify(ctx, sig, msg_hash, pubkey);
    if(is_signature_valid==1){
        return true;
    } else{
        return false;
    }
}