/*
 * ISC License
 *
 * Copyright (c) 2013-2023
 * Frank Denis <j at pureftpd dot org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <string.h>

#include "sodium/crypto_auth_hmacsha512.h"
#include "sodium/crypto_kdf.h"
#include "crypto_kdf_hkdf_sha512.h"
#include "sodium/randombytes.h"
#include "sodium/utils.h"

int
crypto_kdf_hkdf_sha512_extract(
    unsigned char prk[crypto_kdf_hkdf_sha512_KEYBYTES],
    const unsigned char *salt, size_t salt_len, const unsigned char *ikm,
    size_t ikm_len)
{
    crypto_auth_hmacsha512_state st;

    crypto_auth_hmacsha512_init(&st, salt, salt_len);
    crypto_auth_hmacsha512_update(&st, ikm, ikm_len);
    crypto_auth_hmacsha512_final(&st, prk);
    sodium_memzero(&st, sizeof st);

    return 0;
}

void
crypto_kdf_hkdf_sha512_keygen(unsigned char prk[crypto_kdf_hkdf_sha512_KEYBYTES])
{
    randombytes_buf(prk, crypto_kdf_hkdf_sha512_KEYBYTES);
}

int
crypto_kdf_hkdf_sha512_expand(unsigned char *out, size_t out_len,
                              const char *ctx, size_t ctx_len,
                              const unsigned char prk[crypto_kdf_hkdf_sha512_KEYBYTES])
{
    crypto_auth_hmacsha512_state st;
    unsigned char                tmp[crypto_auth_hmacsha512_BYTES];
    size_t                       i;
    size_t                       left;
    unsigned char                counter = 1U;

    if (out_len > crypto_kdf_hkdf_sha512_BYTES_MAX) {
        errno = EINVAL;
        return -1;
    }
    for (i = (size_t) 0U; i + crypto_auth_hmacsha512_BYTES <= out_len;
         i += crypto_auth_hmacsha512_BYTES) {
        crypto_auth_hmacsha512_init(&st, prk, crypto_kdf_hkdf_sha512_KEYBYTES);
        if (i != (size_t) 0U) {
            crypto_auth_hmacsha512_update(&st,
                                          &out[i - crypto_auth_hmacsha512_BYTES],
                                          crypto_auth_hmacsha512_BYTES);
        }
        crypto_auth_hmacsha512_update(&st,
                                      (const unsigned char *) ctx, ctx_len);
        crypto_auth_hmacsha512_update(&st, &counter, (size_t) 1U);
        crypto_auth_hmacsha512_final(&st, &out[i]);
        counter++;
    }
    if ((left = out_len & (crypto_auth_hmacsha512_BYTES - 1U)) != (size_t) 0U) {
        crypto_auth_hmacsha512_init(&st, prk, crypto_kdf_hkdf_sha512_KEYBYTES);
        if (i != (size_t) 0U) {
            crypto_auth_hmacsha512_update(&st,
                                          &out[i - crypto_auth_hmacsha512_BYTES],
                                          crypto_auth_hmacsha512_BYTES);
        }
        crypto_auth_hmacsha512_update(&st,
                                      (const unsigned char *) ctx, ctx_len);
        crypto_auth_hmacsha512_update(&st, &counter, (size_t) 1U);
        crypto_auth_hmacsha512_final(&st, tmp);
        memcpy(&out[i], tmp, left);
        sodium_memzero(tmp, sizeof tmp);
    }
    sodium_memzero(&st, sizeof st);

    return 0;
}

size_t
crypto_kdf_hkdf_sha512_keybytes(void)
{
    return crypto_kdf_hkdf_sha512_KEYBYTES;
}

size_t
crypto_kdf_hkdf_sha512_bytes_min(void)
{
    return crypto_kdf_hkdf_sha512_BYTES_MIN;
}

size_t
crypto_kdf_hkdf_sha512_bytes_max(void)
{
    return crypto_kdf_hkdf_sha512_BYTES_MAX;
}
