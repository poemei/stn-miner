#ifndef STN_GPU_BACKEND_PLATFORM_H
#define STN_GPU_BACKEND_PLATFORM_H

#include <stdint.h>

#include "stn_gpu_backend.h"

stn_gpu_backend_status stn_gpu_backend_platform_prepare(
    const char *kernel_source
);

stn_gpu_backend_status stn_gpu_backend_platform_search(
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
);

#endif
