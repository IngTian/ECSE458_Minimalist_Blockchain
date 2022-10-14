//#include "cryptography.h"
//
//#include <assert.h>
//#include <secp256k1.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include <string.h>
//
//bool verify(){
//    secp256k1_ecdsa_verify()
//}
//
//
//void create_context(unsigned int flag){
//    secp256k1_context* ctx=secp256k1_context_create(flag);
//    return ctx;
//}
//
//void generate_public_key(secp256k1_context *ctx, secp256k1_pubkey *publicKey, const unsigned char *seckey){
//    secp256k1_ec_pubkey_create(ctx,publicKey,seckey);
//}