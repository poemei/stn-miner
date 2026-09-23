#ifndef STN_GPU_BACKEND_H
#define STN_GPU_BACKEND_H

#include <stdint.h>

#include "stn_compute.h"
#include "stn_miner.h"

typedef enum stn_gpu_backend_status {
    STN_GPU_BACKEND_OK = 0,
    STN_GPU_BACKEND_NO_SOLUTION,
    STN_GPU_BACKEND_UNAVAILABLE,
    STN_GPU_BACKEND_INVALID_ARGUMENT,
    STN_GPU_BACKEND_ERROR
} stn_gpu_backend_status;

int stn_gpu_backend_available(
    const stn_compute_inventory *inventory
);

stn_gpu_backend_status stn_gpu_backend_prepare(void);

stn_gpu_backend_status stn_gpu_backend_search(
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
);

#endif
