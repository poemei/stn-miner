#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_gpu_backend.h"
#include "stn_gpu_backend_platform.h"
#include "stn_hash.h"
#include "stn_protocol.h"

#define STN_GPU_BACKEND_MAX_CHUNK 65536u

static const uint8_t stn_gpu_block_id_domain[] =
    "STN-CHAIN:BLOCK:ID:1";

static uint8_t stn_gpu_midstate_work_id[STNM_WORK_ID_SIZE];
static uint8_t stn_gpu_midstate[32];
static int stn_gpu_midstate_valid = 0;

static uint32_t stn_gpu_rotr32(
    uint32_t value,
    uint32_t shift
)
{
    return
        (value >> shift) |
        (value << (32u - shift));
}

static uint32_t stn_gpu_read_u32_be(
    const uint8_t bytes[4]
)
{
    return
        ((uint32_t) bytes[0] << 24) |
        ((uint32_t) bytes[1] << 16) |
        ((uint32_t) bytes[2] << 8) |
        ((uint32_t) bytes[3]);
}

static void stn_gpu_write_u32_be(
    uint8_t bytes[4],
    uint32_t value
)
{
    bytes[0] =
        (uint8_t) (value >> 24);

    bytes[1] =
        (uint8_t) (value >> 16);

    bytes[2] =
        (uint8_t) (value >> 8);

    bytes[3] =
        (uint8_t) value;
}

static void stn_gpu_midstate_transform(
    uint32_t state[8],
    const uint8_t block[64]
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
        words[i] =
            stn_gpu_read_u32_be(
                &block[i * 4u]
            );
    }

    for (i = 16u; i < 64u; ++i) {
        uint32_t s0;
        uint32_t s1;

        s0 =
            stn_gpu_rotr32(
                words[i - 15u],
                7u
            ) ^
            stn_gpu_rotr32(
                words[i - 15u],
                18u
            ) ^
            (words[i - 15u] >> 3u);

        s1 =
            stn_gpu_rotr32(
                words[i - 2u],
                17u
            ) ^
            stn_gpu_rotr32(
                words[i - 2u],
                19u
            ) ^
            (words[i - 2u] >> 10u);

        words[i] =
            words[i - 16u] +
            s0 +
            words[i - 7u] +
            s1;
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    f = state[5];
    g = state[6];
    h = state[7];

    for (i = 0u; i < 64u; ++i) {
        uint32_t s1;
        uint32_t choice;
        uint32_t temp1;
        uint32_t s0;
        uint32_t majority;
        uint32_t temp2;

        s1 =
            stn_gpu_rotr32(e, 6u) ^
            stn_gpu_rotr32(e, 11u) ^
            stn_gpu_rotr32(e, 25u);

        choice =
            (e & f) ^
            ((~e) & g);

        temp1 =
            h +
            s1 +
            choice +
            constants[i] +
            words[i];

        s0 =
            stn_gpu_rotr32(a, 2u) ^
            stn_gpu_rotr32(a, 13u) ^
            stn_gpu_rotr32(a, 22u);

        majority =
            (a & b) ^
            (a & c) ^
            (b & c);

        temp2 =
            s0 +
            majority;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

static int stn_gpu_prepare_midstate(
    const stn_miner_job *job
)
{
    uint8_t prefix[128];
    uint32_t state[8];
    size_t i;

    if (job == NULL ||
        job->block == NULL ||
        job->block_length < STNM_BLOCK_HEADER_LENGTH) {
        return 0;
    }

    if (stn_gpu_midstate_valid &&
        memcmp(
            stn_gpu_midstate_work_id,
            job->work_id,
            STNM_WORK_ID_SIZE
        ) == 0) {
        return 1;
    }

    memcpy(
        prefix,
        stn_gpu_block_id_domain,
        sizeof(stn_gpu_block_id_domain)
    );

    memcpy(
        &prefix[
            sizeof(stn_gpu_block_id_domain)
        ],
        job->block,
        sizeof(prefix) -
            sizeof(stn_gpu_block_id_domain)
    );

    state[0] = 0x6a09e667u;
    state[1] = 0xbb67ae85u;
    state[2] = 0x3c6ef372u;
    state[3] = 0xa54ff53au;
    state[4] = 0x510e527fu;
    state[5] = 0x9b05688cu;
    state[6] = 0x1f83d9abu;
    state[7] = 0x5be0cd19u;

    stn_gpu_midstate_transform(
        state,
        &prefix[0]
    );

    stn_gpu_midstate_transform(
        state,
        &prefix[64]
    );

    for (i = 0u; i < 8u; ++i) {
        stn_gpu_write_u32_be(
            &stn_gpu_midstate[
                i * 4u
            ],
            state[i]
        );
    }

    memcpy(
        stn_gpu_midstate_work_id,
        job->work_id,
        STNM_WORK_ID_SIZE
    );

    stn_gpu_midstate_valid = 1;

    return 1;
}

static const char stn_gpu_opencl_source[] =
    "__constant uint k[64]={"
    "0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,"
    "0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,"
    "0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,"
    "0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,"
    "0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,"
    "0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,"
    "0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,"
    "0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,"
    "0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,"
    "0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,"
    "0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,"
    "0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,"
    "0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,"
    "0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,"
    "0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,"
    "0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};"
    "uint rr(uint x,uint n){return (x>>n)|(x<<(32u-n));}"
    "void cp(uint s[8],uint w[16]){"
    "uint a=s[0],b=s[1],c=s[2],d=s[3];"
    "uint e=s[4],f=s[5],g=s[6],h=s[7];"
    "uint i;"
    "for(i=0u;i<64u;i++){"
    "uint wi;"
    "if(i<16u){wi=w[i];}else{"
    "uint x=w[(i+1u)&15u],y=w[(i+14u)&15u];"
    "uint z0=rr(x,7u)^rr(x,18u)^(x>>3u);"
    "uint z1=rr(y,17u)^rr(y,19u)^(y>>10u);"
    "uint q=i&15u;"
    "w[q]=w[q]+z0+w[(i+9u)&15u]+z1;"
    "wi=w[q];}"
    "uint S1=rr(e,6u)^rr(e,11u)^rr(e,25u);"
    "uint ch=(e&f)^((~e)&g);"
    "uint t1=h+S1+ch+k[i]+wi;"
    "uint S0=rr(a,2u)^rr(a,13u)^rr(a,22u);"
    "uint maj=(a&b)^(a&c)^(b&c);"
    "uint t2=S0+maj;"
    "h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}"
    "s[0]+=a;s[1]+=b;s[2]+=c;s[3]+=d;"
    "s[4]+=e;s[5]+=f;s[6]+=g;s[7]+=h;}"
    "__kernel void stn_mine("
    "__global const uchar *header,"
    "__global const uchar *target,"
    "ulong nonce_start,"
    "__global uchar *matches,"
    "__global const uchar *mid){"
    "size_t gid=get_global_id(0);"
    "ulong nonce=nonce_start+(ulong)gid;"
    "uint s[8],w[16],i;"
    "uchar b[64];"
    "for(i=0u;i<8u;i++){uint q=i*4u;"
    "s[i]=((uint)mid[q]<<24)|((uint)mid[q+1]<<16)|"
    "((uint)mid[q+2]<<8)|(uint)mid[q+3];}"
    "for(i=0u;i<61u;i++)b[i]=header[107u+i];"
    "b[45]=(uchar)(nonce>>56);b[46]=(uchar)(nonce>>48);"
    "b[47]=(uchar)(nonce>>40);b[48]=(uchar)(nonce>>32);"
    "b[49]=(uchar)(nonce>>24);b[50]=(uchar)(nonce>>16);"
    "b[51]=(uchar)(nonce>>8);b[52]=(uchar)nonce;"
    "b[61]=0x80;b[62]=0;b[63]=0;"
    "for(i=0u;i<16u;i++){uint q=i*4u;"
    "w[i]=((uint)b[q]<<24)|((uint)b[q+1]<<16)|"
    "((uint)b[q+2]<<8)|(uint)b[q+3];}"
    "cp(s,w);"
    "for(i=0u;i<15u;i++)w[i]=0u;"
    "w[15]=0x000005e8u;"
    "cp(s,w);"
    "matches[gid]=1;"
    "for(i=0u;i<8u;i++){uint q=i*4u;"
    "uint tv=((uint)target[q]<<24)|((uint)target[q+1]<<16)|"
    "((uint)target[q+2]<<8)|(uint)target[q+3];"
    "if(s[i]<tv)break;"
    "if(s[i]>tv){matches[gid]=0;break;}}"
    "}";

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

    if (!stn_gpu_prepare_midstate(
            job
        )) {
        return STN_GPU_BACKEND_ERROR;
    }

    status =
        stn_gpu_backend_platform_search(
            job,
            stn_gpu_midstate,
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
