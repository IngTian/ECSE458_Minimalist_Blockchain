#ifndef MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H
#define MINIMALIST_BLOCK_CHAIN_SYSTEM_SRC_UTILS_CRYPTOGRAPHY_H

#include <secp256k1.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

/**    sha256 functions   **/
char *hash_struct(void *, unsigned int);

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

/*************************************************************************
 * Copyright (c) 2020-2021 Elichai Turkel                                *
 * Distributed under the CC0 software license, see the accompanying file *
 * EXAMPLES_COPYING or https://creativecommons.org/publicdomain/zero/1.0 *
 *************************************************************************/

/*
 * This file is an attempt at collecting best practice methods for obtaining randomness with different operating
 * systems. It may be out-of-date. Consult the documentation of the operating system before considering to use the
 * methods below.
 *
 * Platform randomness sources:
 * Linux   -> `getrandom(2)`(`sys/random.h`), if not available `/dev/urandom` should be used.
 * http://man7.org/linux/man-pages/man2/getrandom.2.html, https://linux.die.net/man/4/urandom macOS   ->
 * `getentropy(2)`(`sys/random.h`), if not available `/dev/urandom` should be used.
 * https://www.unix.com/man-page/mojave/2/getentropy,
 * https://opensource.apple.com/source/xnu/xnu-517.12.7/bsd/man/man4/random.4.auto.html FreeBSD ->
 * `getrandom(2)`(`sys/random.h`), if not available `kern.arandom` should be used.
 * https://www.freebsd.org/cgi/man.cgi?query=getrandom, https://www.freebsd.org/cgi/man.cgi?query=random&sektion=4
 * OpenBSD -> `getentropy(2)`(`unistd.h`), if not available `/dev/urandom` should be used.
 * https://man.openbsd.org/getentropy, https://man.openbsd.org/urandom Windows -> `BCryptGenRandom`(`bcrypt.h`).
 * https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom
 */

#if defined(_WIN32)
#include <bcrypt.h>
#include <ntstatus.h>
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/random.h>
#elif defined(__OpenBSD__)
#include <unistd.h>
#else
#error "Couldn't identify the OS"
#endif

#include <limits.h>
#include <stddef.h>
#include <stdio.h>

/* Returns 1 on success, and 0 on failure. */
static int fill_random(unsigned char *data, size_t size) {
#if defined(_WIN32)
    NTSTATUS res = BCryptGenRandom(NULL, data, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (res != STATUS_SUCCESS || size > ULONG_MAX) {
        return 0;
    } else {
        return 1;
    }
#elif defined(__linux__) || defined(__FreeBSD__)
    /* If `getrandom(2)` is not available you should fallback to /dev/urandom */
    ssize_t res = getrandom(data, size, 0);
    if (res < 0 || (size_t)res != size) {
        return 0;
    } else {
        return 1;
    }
#elif defined(__APPLE__) || defined(__OpenBSD__)
    /* If `getentropy(2)` is not available you should fallback to either
     * `SecRandomCopyBytes` or /dev/urandom */
    int res = getentropy(data, size);
    if (res == 0) {
        return 1;
    } else {
        return 0;
    }
#endif
    return 0;
}

#endif
