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
    "__constant uchar dom[21]={"
    "0x53,0x54,0x4e,0x2d,0x43,0x48,0x41,0x49,0x4e,0x3a,"
    "0x42,0x4c,0x4f,0x43,0x4b,0x3a,0x49,0x44,0x3a,0x31,0};"
    "uint rr(uint x,uint n){return (x>>n)|(x<<(32u-n));}"
    "__kernel void stn_mine("
    "__global const uchar *header,"
    "__global const uchar *target,"
    "ulong nonce_start,"
    "__global uchar *matches){"
    "size_t gid=get_global_id(0);"
    "ulong nonce=nonce_start+(ulong)gid;"
    "uchar m[256];uchar dg[32];uint w[64];uint s[8];uint i,j;"
    "for(i=0u;i<21u;i++)m[i]=dom[i];"
    "for(i=0u;i<168u;i++)m[21u+i]=header[i];"
    "m[173]=(uchar)(nonce>>56);m[174]=(uchar)(nonce>>48);"
    "m[175]=(uchar)(nonce>>40);m[176]=(uchar)(nonce>>32);"
    "m[177]=(uchar)(nonce>>24);m[178]=(uchar)(nonce>>16);"
    "m[179]=(uchar)(nonce>>8);m[180]=(uchar)nonce;"
    "m[189]=0x80;"
    "for(i=190u;i<256u;i++)m[i]=0;"
    "m[254]=0x05;m[255]=0xe8;"
    "s[0]=0x6a09e667u;s[1]=0xbb67ae85u;"
    "s[2]=0x3c6ef372u;s[3]=0xa54ff53au;"
    "s[4]=0x510e527fu;s[5]=0x9b05688cu;"
    "s[6]=0x1f83d9abu;s[7]=0x5be0cd19u;"
    "for(j=0u;j<4u;j++){"
    "uint a,b,c,d,e,f,g,h;"
    "for(i=0u;i<16u;i++){uint q=j*64u+i*4u;"
    "w[i]=((uint)m[q]<<24)|((uint)m[q+1]<<16)|"
    "((uint)m[q+2]<<8)|(uint)m[q+3];}"
    "for(i=16u;i<64u;i++){"
    "uint x=w[i-15u],y=w[i-2u];"
    "uint z0=rr(x,7u)^rr(x,18u)^(x>>3u);"
    "uint z1=rr(y,17u)^rr(y,19u)^(y>>10u);"
    "w[i]=w[i-16u]+z0+w[i-7u]+z1;}"
    "a=s[0];b=s[1];c=s[2];d=s[3];"
    "e=s[4];f=s[5];g=s[6];h=s[7];"
    "for(i=0u;i<64u;i++){"
    "uint S1=rr(e,6u)^rr(e,11u)^rr(e,25u);"
    "uint ch=(e&f)^((~e)&g);"
    "uint t1=h+S1+ch+k[i]+w[i];"
    "uint S0=rr(a,2u)^rr(a,13u)^rr(a,22u);"
    "uint maj=(a&b)^(a&c)^(b&c);"
    "uint t2=S0+maj;"
    "h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}"
    "s[0]+=a;s[1]+=b;s[2]+=c;s[3]+=d;"
    "s[4]+=e;s[5]+=f;s[6]+=g;s[7]+=h;}"
    "for(i=0u;i<8u;i++){uint v=s[i];"
    "dg[i*4u]=(uchar)(v>>24);"
    "dg[i*4u+1u]=(uchar)(v>>16);"
    "dg[i*4u+2u]=(uchar)(v>>8);"
    "dg[i*4u+3u]=(uchar)v;}"
    "matches[gid]=1;"
    "for(i=0u;i<32u;i++){"
    "if(dg[i]<target[i])break;"
    "if(dg[i]>target[i]){matches[gid]=0;break;}}"
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
