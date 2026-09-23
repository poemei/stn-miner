#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_gpu_backend.h"
#include "stn_gpu_backend_platform.h"
#include "stn_hash.h"
#include "stn_protocol.h"

#define STN_GPU_BACKEND_MAX_CHUNK 4096u

static const uint8_t stn_gpu_block_id_domain[] =
    "STN-CHAIN:BLOCK:ID:1";

static const char stn_gpu_opencl_source[] =
"__constant uint STN_K[64] = {\n"
"  0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,\n"
"  0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,\n"
"  0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,\n"
"  0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,\n"
"  0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,\n"
"  0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,\n"
"  0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,\n"
"  0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,\n"
"  0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,\n"
"  0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,\n"
"  0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,\n"
"  0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,\n"
"  0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,\n"
"  0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,\n"
"  0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,\n"
"  0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U\n"
"};\n"
"\n"
"__constant uchar STN_DOMAIN[20] = {\n"
"  'S','T','N','-','C','H','A','I','N',':',\n"
"  'B','L','O','C','K',':','I','D',':','1'\n"
"};\n"
"\n"
"uint stn_rotr(uint x, uint n)\n"
"{\n"
"    return (x >> n) | (x << (32U - n));\n"
"}\n"
"\n"
"uint stn_load_be32_private(const uchar *p)\n"
"{\n"
"    return ((uint)p[0] << 24) |\n"
"           ((uint)p[1] << 16) |\n"
"           ((uint)p[2] << 8) |\n"
"           ((uint)p[3]);\n"
"}\n"
"\n"
"uint stn_load_be32_global(__global const uchar *p)\n"
"{\n"
"    return ((uint)p[0] << 24) |\n"
"           ((uint)p[1] << 16) |\n"
"           ((uint)p[2] << 8) |\n"
"           ((uint)p[3]);\n"
"}\n"
"\n"
"void stn_sha256_compress(\n"
"    const uchar *block,\n"
"    uint state[8]\n"
")\n"
"{\n"
"    uint w[64];\n"
"    uint a;\n"
"    uint b;\n"
"    uint c;\n"
"    uint d;\n"
"    uint e;\n"
"    uint f;\n"
"    uint g;\n"
"    uint h;\n"
"    uint s0;\n"
"    uint s1;\n"
"    uint ch;\n"
"    uint maj;\n"
"    uint t1;\n"
"    uint t2;\n"
"    uint i;\n"
"\n"
"    for (i = 0U; i < 16U; ++i) {\n"
"        w[i] =\n"
"            stn_load_be32_private(\n"
"                block + (i * 4U)\n"
"            );\n"
"    }\n"
"\n"
"    for (i = 16U; i < 64U; ++i) {\n"
"        s0 =\n"
"            stn_rotr(w[i - 15U], 7U) ^\n"
"            stn_rotr(w[i - 15U], 18U) ^\n"
"            (w[i - 15U] >> 3U);\n"
"\n"
"        s1 =\n"
"            stn_rotr(w[i - 2U], 17U) ^\n"
"            stn_rotr(w[i - 2U], 19U) ^\n"
"            (w[i - 2U] >> 10U);\n"
"\n"
"        w[i] =\n"
"            w[i - 16U] +\n"
"            s0 +\n"
"            w[i - 7U] +\n"
"            s1;\n"
"    }\n"
"\n"
"    a = state[0];\n"
"    b = state[1];\n"
"    c = state[2];\n"
"    d = state[3];\n"
"    e = state[4];\n"
"    f = state[5];\n"
"    g = state[6];\n"
"    h = state[7];\n"
"\n"
"    for (i = 0U; i < 64U; ++i) {\n"
"        s1 =\n"
"            stn_rotr(e, 6U) ^\n"
"            stn_rotr(e, 11U) ^\n"
"            stn_rotr(e, 25U);\n"
"\n"
"        ch =\n"
"            (e & f) ^\n"
"            ((~e) & g);\n"
"\n"
"        t1 =\n"
"            h +\n"
"            s1 +\n"
"            ch +\n"
"            STN_K[i] +\n"
"            w[i];\n"
"\n"
"        s0 =\n"
"            stn_rotr(a, 2U) ^\n"
"            stn_rotr(a, 13U) ^\n"
"            stn_rotr(a, 22U);\n"
"\n"
"        maj =\n"
"            (a & b) ^\n"
"            (a & c) ^\n"
"            (b & c);\n"
"\n"
"        t2 = s0 + maj;\n"
"\n"
"        h = g;\n"
"        g = f;\n"
"        f = e;\n"
"        e = d + t1;\n"
"        d = c;\n"
"        c = b;\n"
"        b = a;\n"
"        a = t1 + t2;\n"
"    }\n"
"\n"
"    state[0] += a;\n"
"    state[1] += b;\n"
"    state[2] += c;\n"
"    state[3] += d;\n"
"    state[4] += e;\n"
"    state[5] += f;\n"
"    state[6] += g;\n"
"    state[7] += h;\n"
"}\n"
"\n"
"int stn_hash_meets_target(\n"
"    const uint state[8],\n"
"    __global const uchar *target\n"
")\n"
"{\n"
"    uint i;\n"
"\n"
"    for (i = 0U; i < 8U; ++i) {\n"
"        uint target_word =\n"
"            stn_load_be32_global(\n"
"                target + (i * 4U)\n"
"            );\n"
"\n"
"        if (state[i] < target_word) {\n"
"            return 1;\n"
"        }\n"
"\n"
"        if (state[i] > target_word) {\n"
"            return 0;\n"
"        }\n"
"    }\n"
"\n"
"    return 1;\n"
"}\n"
"\n"
"__kernel void stn_mine(\n"
"    __global const uchar *header,\n"
"    __global const uchar *target,\n"
"    ulong nonce_start,\n"
"    ulong nonce_count,\n"
"    __global uint *matches\n"
")\n"
"{\n"
"    size_t gid = get_global_id(0);\n"
"    ulong nonce;\n"
"    uchar message[256];\n"
"    uint state[8];\n"
"    uint i;\n"
"\n"
"    if ((ulong)gid >= nonce_count) {\n"
"        return;\n"
"    }\n"
"\n"
"    nonce = nonce_start + (ulong)gid;\n"
"\n"
"    for (i = 0U; i < 256U; ++i) {\n"
"        message[i] = (uchar)0;\n"
"    }\n"
"\n"
"    for (i = 0U; i < 20U; ++i) {\n"
"        message[i] = STN_DOMAIN[i];\n"
"    }\n"
"\n"
"    message[20] = (uchar)0;\n"
"\n"
"    for (i = 0U; i < 168U; ++i) {\n"
"        message[21U + i] = header[i];\n"
"    }\n"
"\n"
"    /*\n"
"     * Original header nonce offset 152 becomes\n"
"     * message offset 21 + 152 = 173.\n"
"     */\n"
"    message[173] = (uchar)(nonce >> 56);\n"
"    message[174] = (uchar)(nonce >> 48);\n"
"    message[175] = (uchar)(nonce >> 40);\n"
"    message[176] = (uchar)(nonce >> 32);\n"
"    message[177] = (uchar)(nonce >> 24);\n"
"    message[178] = (uchar)(nonce >> 16);\n"
"    message[179] = (uchar)(nonce >> 8);\n"
"    message[180] = (uchar)(nonce);\n"
"\n"
"    /*\n"
"     * SHA-256 message length:\n"
"     *\n"
"     * 20 domain bytes\n"
"     *  1 separator byte\n"
"     * 168 header bytes\n"
"     * ----------------\n"
"     * 189 bytes\n"
"     */\n"
"    message[189] = (uchar)0x80;\n"
"\n"
"    /* 189 * 8 = 1512 bits = 0x05e8 */\n"
"    message[248] = (uchar)0x00;\n"
"    message[249] = (uchar)0x00;\n"
"    message[250] = (uchar)0x00;\n"
"    message[251] = (uchar)0x00;\n"
"    message[252] = (uchar)0x00;\n"
"    message[253] = (uchar)0x00;\n"
"    message[254] = (uchar)0x05;\n"
"    message[255] = (uchar)0xe8;\n"
"\n"
"    state[0] = 0x6a09e667U;\n"
"    state[1] = 0xbb67ae85U;\n"
"    state[2] = 0x3c6ef372U;\n"
"    state[3] = 0xa54ff53aU;\n"
"    state[4] = 0x510e527fU;\n"
"    state[5] = 0x9b05688cU;\n"
"    state[6] = 0x1f83d9abU;\n"
"    state[7] = 0x5be0cd19U;\n"
"\n"
"    stn_sha256_compress(\n"
"        message + 0U,\n"
"        state\n"
"    );\n"
"\n"
"    stn_sha256_compress(\n"
"        message + 64U,\n"
"        state\n"
"    );\n"
"\n"
"    stn_sha256_compress(\n"
"        message + 128U,\n"
"        state\n"
"    );\n"
"\n"
"    stn_sha256_compress(\n"
"        message + 192U,\n"
"        state\n"
"    );\n"
"\n"
"    matches[gid] =\n"
"        stn_hash_meets_target(\n"
"            state,\n"
"            target\n"
"        )\n"
"            ? 1U\n"
"            : 0U;\n"
"}\n";

static int stn_gpu_backend_verify(
    const stn_miner_job *job,
    uint64_t nonce
)
{
    uint8_t header[STNM_BLOCK_HEADER_LENGTH];
    uint8_t digest[STN_SHA256_DIGEST_SIZE];
    stn_sha256_context context;

    if (job == NULL ||
        job->block == NULL ||
        job->block_length < STNM_BLOCK_HEADER_LENGTH) {
        return 0;
    }

    memcpy(
        header,
        job->block,
        sizeof(header)
    );

    stn_protocol_write_u64_be(
        &header[STNM_BLOCK_NONCE_OFFSET],
        nonce
    );

    stn_sha256_init(
        &context
    );

    stn_sha256_update(
        &context,
        stn_gpu_block_id_domain,
        sizeof(stn_gpu_block_id_domain)
    );

    stn_sha256_update(
        &context,
        header,
        sizeof(header)
    );

    stn_sha256_final(
        &context,
        digest
    );

    return stn_hash_meets_target(
        digest,
        job->target
    );
}

int stn_gpu_backend_available(
    const stn_compute_inventory *inventory
)
{
    size_t i;

    if (inventory == NULL) {
        return 0;
    }

    for (i = 0u;
         i < inventory->count;
         ++i) {

        if (inventory->providers[i].type ==
                STN_COMPUTE_PROVIDER_OPENCL &&
            inventory->providers[i].sha256_ready) {
            return 1;
        }
    }

    return 0;
}

stn_gpu_backend_status stn_gpu_backend_prepare(void)
{
    return stn_gpu_backend_platform_prepare(
        stn_gpu_opencl_source
    );
}

stn_gpu_backend_status stn_gpu_backend_search(
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
)
{
    stn_gpu_backend_status status;
    uint64_t count;

    if (job == NULL ||
        solution == NULL ||
        job->block == NULL ||
        job->block_length < STNM_BLOCK_HEADER_LENGTH ||
        nonce_start > nonce_end) {
        return STN_GPU_BACKEND_INVALID_ARGUMENT;
    }

    count =
        (nonce_end - nonce_start) + 1u;

    if (count == 0u ||
        count > STN_GPU_BACKEND_MAX_CHUNK) {
        return STN_GPU_BACKEND_INVALID_ARGUMENT;
    }

    status =
        stn_gpu_backend_platform_search(
            job,
            nonce_start,
            nonce_end,
            solution
        );

    if (status != STN_GPU_BACKEND_OK) {
        return status;
    }

    if (!stn_gpu_backend_verify(
            job,
            solution->nonce
        )) {
        return STN_GPU_BACKEND_ERROR;
    }

    return STN_GPU_BACKEND_OK;
}
