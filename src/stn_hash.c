/* C:\poes_projects\stn-miner\src\stn_hash.c */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_hash.h"

static uint32_t stn_rotr32(
    uint32_t value,
    uint32_t shift
)
{
    return (value >> shift) | (value << (32u - shift));
}

static uint32_t stn_read_u32_be(
    const uint8_t bytes[4]
)
{
    return
        ((uint32_t) bytes[0] << 24) |
        ((uint32_t) bytes[1] << 16) |
        ((uint32_t) bytes[2] << 8) |
        ((uint32_t) bytes[3]);
}

static void stn_write_u32_be(
    uint8_t bytes[4],
    uint32_t value
)
{
    bytes[0] = (uint8_t) ((value >> 24) & 0xffu);
    bytes[1] = (uint8_t) ((value >> 16) & 0xffu);
    bytes[2] = (uint8_t) ((value >> 8) & 0xffu);
    bytes[3] = (uint8_t) (value & 0xffu);
}

static void stn_write_u64_be(
    uint8_t bytes[8],
    uint64_t value
)
{
    bytes[0] = (uint8_t) ((value >> 56) & 0xffu);
    bytes[1] = (uint8_t) ((value >> 48) & 0xffu);
    bytes[2] = (uint8_t) ((value >> 40) & 0xffu);
    bytes[3] = (uint8_t) ((value >> 32) & 0xffu);
    bytes[4] = (uint8_t) ((value >> 24) & 0xffu);
    bytes[5] = (uint8_t) ((value >> 16) & 0xffu);
    bytes[6] = (uint8_t) ((value >> 8) & 0xffu);
    bytes[7] = (uint8_t) (value & 0xffu);
}

static void stn_sha256_transform(
    stn_sha256_context *context,
    const uint8_t block[STN_SHA256_BLOCK_SIZE]
)
{
    static const uint32_t constants[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
        0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
        0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
        0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
        0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
        0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
        0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
        0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
        0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    };

    uint32_t words[64];
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f;
    uint32_t g;
    uint32_t h;
    size_t i;

    for (i = 0u; i < 16u; ++i) {
        words[i] = stn_read_u32_be(&block[i * 4u]);
    }

    for (i = 16u; i < 64u; ++i) {
        uint32_t s0;
        uint32_t s1;

        s0 =
            stn_rotr32(words[i - 15u], 7u) ^
            stn_rotr32(words[i - 15u], 18u) ^
            (words[i - 15u] >> 3u);

        s1 =
            stn_rotr32(words[i - 2u], 17u) ^
            stn_rotr32(words[i - 2u], 19u) ^
            (words[i - 2u] >> 10u);

        words[i] =
            words[i - 16u] +
            s0 +
            words[i - 7u] +
            s1;
    }

    a = context->state[0];
    b = context->state[1];
    c = context->state[2];
    d = context->state[3];
    e = context->state[4];
    f = context->state[5];
    g = context->state[6];
    h = context->state[7];

    for (i = 0u; i < 64u; ++i) {
        uint32_t s1;
        uint32_t choice;
        uint32_t temp1;
        uint32_t s0;
        uint32_t majority;
        uint32_t temp2;

        s1 =
            stn_rotr32(e, 6u) ^
            stn_rotr32(e, 11u) ^
            stn_rotr32(e, 25u);

        choice = (e & f) ^ ((~e) & g);

        temp1 =
            h +
            s1 +
            choice +
            constants[i] +
            words[i];

        s0 =
            stn_rotr32(a, 2u) ^
            stn_rotr32(a, 13u) ^
            stn_rotr32(a, 22u);

        majority = (a & b) ^ (a & c) ^ (b & c);

        temp2 = s0 + majority;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
    context->state[5] += f;
    context->state[6] += g;
    context->state[7] += h;
}

void stn_sha256_init(
    stn_sha256_context *context
)
{
    if (context == NULL) {
        return;
    }

    context->state[0] = 0x6a09e667u;
    context->state[1] = 0xbb67ae85u;
    context->state[2] = 0x3c6ef372u;
    context->state[3] = 0xa54ff53au;
    context->state[4] = 0x510e527fu;
    context->state[5] = 0x9b05688cu;
    context->state[6] = 0x1f83d9abu;
    context->state[7] = 0x5be0cd19u;

    context->bit_count = 0u;
    context->buffer_length = 0u;

    memset(
        context->buffer,
        0,
        sizeof(context->buffer)
    );
}

void stn_sha256_update(
    stn_sha256_context *context,
    const uint8_t *data,
    size_t length
)
{
    size_t offset;

    if (context == NULL || (data == NULL && length != 0u)) {
        return;
    }

    offset = 0u;

    while (offset < length) {
        size_t available;
        size_t remaining;
        size_t copy_length;

        available =
            STN_SHA256_BLOCK_SIZE -
            context->buffer_length;

        remaining = length - offset;

        copy_length =
            (remaining < available)
                ? remaining
                : available;

        memcpy(
            &context->buffer[context->buffer_length],
            &data[offset],
            copy_length
        );

        context->buffer_length += copy_length;
        offset += copy_length;

        if (context->buffer_length == STN_SHA256_BLOCK_SIZE) {
            stn_sha256_transform(
                context,
                context->buffer
            );

            context->buffer_length = 0u;
        }
    }

    context->bit_count += ((uint64_t) length * 8u);
}

void stn_sha256_final(
    stn_sha256_context *context,
    uint8_t digest[STN_SHA256_DIGEST_SIZE]
)
{
    uint8_t length_bytes[8];
    size_t i;

    if (context == NULL || digest == NULL) {
        return;
    }

    stn_write_u64_be(
        length_bytes,
        context->bit_count
    );

    context->buffer[context->buffer_length++] = 0x80u;

    if (context->buffer_length > 56u) {
        while (context->buffer_length < STN_SHA256_BLOCK_SIZE) {
            context->buffer[context->buffer_length++] = 0u;
        }

        stn_sha256_transform(
            context,
            context->buffer
        );

        context->buffer_length = 0u;
    }

    while (context->buffer_length < 56u) {
        context->buffer[context->buffer_length++] = 0u;
    }

    memcpy(
        &context->buffer[56],
        length_bytes,
        sizeof(length_bytes)
    );

    stn_sha256_transform(
        context,
        context->buffer
    );

    for (i = 0u; i < 8u; ++i) {
        stn_write_u32_be(
            &digest[i * 4u],
            context->state[i]
        );
    }

    memset(
        context,
        0,
        sizeof(*context)
    );
}

void stn_sha256(
    const uint8_t *data,
    size_t length,
    uint8_t digest[STN_SHA256_DIGEST_SIZE]
)
{
    stn_sha256_context context;

    if (digest == NULL) {
        return;
    }

    stn_sha256_init(&context);
    stn_sha256_update(&context, data, length);
    stn_sha256_final(&context, digest);
}

int stn_hash_meets_target(
    const uint8_t hash[STN_SHA256_DIGEST_SIZE],
    const uint8_t target[STN_SHA256_DIGEST_SIZE]
)
{
    size_t i;

    if (hash == NULL || target == NULL) {
        return 0;
    }

    for (i = 0u; i < STN_SHA256_DIGEST_SIZE; ++i) {
        if (hash[i] < target[i]) {
            return 1;
        }

        if (hash[i] > target[i]) {
            return 0;
        }
    }

    return 1;
}