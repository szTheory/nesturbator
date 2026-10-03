/* SHA-256 (FIPS 180-4), owned code for the runner's frame hashes. */
#ifndef NESTURBATOR_RUN_SHA256_H
#define NESTURBATOR_RUN_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct nesturbator_run_sha256 {
    uint32_t h[8];     /* intermediate hash value */
    uint8_t block[64]; /* bytes of the current, unfinished block */
    uint32_t used;     /* bytes in block */
    uint64_t total;    /* message length in bytes */
} nesturbator_run_sha256;

void nesturbator_run_sha256_init(nesturbator_run_sha256 *s);
void nesturbator_run_sha256_update(nesturbator_run_sha256 *s, const void *data, size_t len);
void nesturbator_run_sha256_final(nesturbator_run_sha256 *s, uint8_t digest[32]);

/* Writes the digest as 64 lowercase hex digits and a terminating NUL. */
void nesturbator_run_sha256_hex(const uint8_t digest[32], char hex[65]);

#endif /* NESTURBATOR_RUN_SHA256_H */
