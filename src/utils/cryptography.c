#include "cryptography.h"

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/log_utils.h"

// secp256k1 global variable
secp256k1_context *g_crypto_context;

// SHA256 data types
typedef struct {
    unsigned char data[64];
    unsigned int data_len;
    unsigned long long bit_len;
    unsigned int state[8];
} SHA256_CTX;

char *hash_sha256(char *);
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const unsigned char data[], size_t len);
void sha256_final(SHA256_CTX *ctx, unsigned char hash[]);

// SHA256 macro
#define ROT_LEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define ROT_RIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROT_RIGHT(x, 2) ^ ROT_RIGHT(x, 13) ^ ROT_RIGHT(x, 22))
#define EP1(x) (ROT_RIGHT(x, 6) ^ ROT_RIGHT(x, 11) ^ ROT_RIGHT(x, 25))
#define SIG0(x) (ROT_RIGHT(x, 7) ^ ROT_RIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROT_RIGHT(x, 17) ^ ROT_RIGHT(x, 19) ^ ((x) >> 10))

// SHA256 global variable
static const unsigned int g_sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

SHA256_CTX *g_sha256_ctx;

/*
 * -----------------------------------------------------------
 * Encryption (secp256k1)
 * -----------------------------------------------------------
 */

/**
 * Initialize the crypto system.
 * @param flag
 * @author Junjian Chen
 */
void initialize_cryptography_system(unsigned int flag) {
    g_crypto_context = secp256k1_context_create(flag);
    g_sha256_ctx = malloc(sizeof(SHA256_CTX));
    sha256_init(g_sha256_ctx);
}

/**
 * Destroy the cryptography system.
 * @author Junjian Chen
 */
void destroy_cryptography_system() {
    secp256k1_context_destroy(g_crypto_context);
    free(g_sha256_ctx);
}

/**
 * Sign a message with a private key.
 * @param private_key A 64-byte private key.
 * @param msg_to_sign A string of message to be signed.
 * @return Signature
 * @author Ing Tian
 */
secp256k1_ecdsa_signature *sign(unsigned char *private_key, unsigned char *msg_to_sign) {
    secp256k1_ecdsa_signature *signature = (secp256k1_ecdsa_signature *)malloc(sizeof(secp256k1_ecdsa_signature));
    secp256k1_ecdsa_sign(g_crypto_context, signature, msg_to_sign, private_key, NULL, NULL);
    return signature;
}

/**
 * Verify the validity of a signature.
 * @param public_key The public key.
 * @param msg_hash The message signed by the private key.
 * @param signature The signature.
 * @return True for valid, and false otherwise.
 * @auhtor Junjian Chen
 */
bool verify(secp256k1_pubkey *public_key, unsigned char *msg_hash, secp256k1_ecdsa_signature *signature) {
    int is_signature_valid = secp256k1_ecdsa_verify(g_crypto_context, signature, msg_hash, public_key);
    if (is_signature_valid == 1) {
        return true;
    } else {
        return false;
    }
}

/**
 * Generate a random private key.
 * @return A new private key (64 bytes).
 * @author Junjian Chen
 */
unsigned char *get_a_new_private_key() {
    unsigned char *private_key = (unsigned char *)malloc(sizeof(char) * 64);
    while (1) {
        if (!fill_random(private_key, 32)) {
            printf("Failed to generate randomness\n");
            return NULL;
        }
        if (secp256k1_ec_seckey_verify(g_crypto_context, private_key)) {
            return private_key;
        }
    }
}

/**
 * Generate a new public key based on
 * @param private_key The private key out of which the public key is generated.
 * @return A new public key.
 * @author Junjian Chen
 */
secp256k1_pubkey *get_a_new_public_key(char *private_key) {
    secp256k1_pubkey *ret_val = (secp256k1_pubkey *)malloc(sizeof(secp256k1_pubkey));
    if (!secp256k1_ec_pubkey_create(g_crypto_context, ret_val, (const unsigned char *)private_key)) return NULL;
    return ret_val;
}

/*
 * -----------------------------------------------------------
 * SHA256
 * -----------------------------------------------------------
 */

/**
 * Return the SHA256 hashcode of a general
 * struct. Note that this will first convert
 * the data in to a hexadecimal format.
 * @param ptr A pointer.
 * @param size The size, in bytes, to hash.
 * @return SHA256 hashcode.
 * @author Ing Tian
 */
char *hash_struct_in_hex(void *ptr, unsigned int size) {
    char *msg = convert_char_hexadecimal(ptr, size);
    char *hash_msg = hash_sha256(msg);
    free(msg);
    char *hash_msg_hex = convert_char_hexadecimal(hash_msg, 32);
    free(hash_msg);
    return hash_msg_hex;
}

/**
 * Hash a string of data.
 * @param data a string
 * @return Return a string of 32 bytes (256 bits)
 * @auhtor Junjian Chen
 */
char *hash_sha256(char *data) {
    char *result = malloc(sizeof(char) * 32);
    sha256_update(g_sha256_ctx, (unsigned char *)data, strlen(data));
    sha256_final(g_sha256_ctx, (unsigned char *)result);
    return result;
}

void sha256_transform(SHA256_CTX *ctx, const unsigned char data[]) {
    unsigned int a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for (; i < 64; ++i) m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + g_sha256_k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
    ctx->data_len = 0;
    ctx->bit_len = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const unsigned char data[], size_t len) {
    unsigned int i;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->data_len] = data[i];
        ctx->data_len++;
        if (ctx->data_len == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bit_len += 512;
            ctx->data_len = 0;
        }
    }
}

void sha256_final(SHA256_CTX *ctx, unsigned char hash[]) {
    unsigned int i;

    i = ctx->data_len;

    // Pad whatever data is left in the buffer.
    if (ctx->data_len < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bit_len += ctx->data_len * 8;
    ctx->data[63] = ctx->bit_len;
    ctx->data[62] = ctx->bit_len >> 8;
    ctx->data[61] = ctx->bit_len >> 16;
    ctx->data[60] = ctx->bit_len >> 24;
    ctx->data[59] = ctx->bit_len >> 32;
    ctx->data[58] = ctx->bit_len >> 40;
    ctx->data[57] = ctx->bit_len >> 48;
    ctx->data[56] = ctx->bit_len >> 56;
    sha256_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}
