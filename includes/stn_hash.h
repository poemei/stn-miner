/* C:\poes_projects\stn-miner\includes\stn_hash.h */

#ifndef STN_HASH_H
#define STN_HASH_H

#include <stddef.h>
#include <stdint.h>

#define STN_SHA256_DIGEST_SIZE 32u
#define STN_SHA256_BLOCK_SIZE 64u

typedef struct stn_sha256_context {
    uint32_t state[8];
    uint64_t bit_count;
    uint8_t buffer[STN_SHA256_BLOCK_SIZE];
    size_t buffer_length;
} stn_sha256_context;

void stn_sha256_init(
    stn_sha256_context *context
);

void stn_sha256_update(
    stn_sha256_context *context,
    const uint8_t *data,
    size_t length
);

void stn_sha256_final(
    stn_sha256_context *context,
    uint8_t digest[STN_SHA256_DIGEST_SIZE]
);

void stn_sha256(
    const uint8_t *data,
    size_t length,
    uint8_t digest[STN_SHA256_DIGEST_SIZE]
);

int stn_hash_meets_target(
    const uint8_t hash[STN_SHA256_DIGEST_SIZE],
    const uint8_t target[STN_SHA256_DIGEST_SIZE]
);

#endif