#ifndef STN_BACKEND_H
#define STN_BACKEND_H

#include <stdint.h>

#include "stn_compute.h"
#include "stn_gpu.h"
#include "stn_miner.h"

typedef enum stn_backend_type {
    STN_BACKEND_TYPE_NONE = 0,
    STN_BACKEND_TYPE_CPU,
    STN_BACKEND_TYPE_GPU,
    STN_BACKEND_TYPE_USB_ASIC,
    STN_BACKEND_TYPE_ASIC
} stn_backend_type;

typedef enum stn_backend_status {
    STN_BACKEND_OK = 0,
    STN_BACKEND_NO_SOLUTION,
    STN_BACKEND_UNAVAILABLE,
    STN_BACKEND_INVALID_ARGUMENT,
    STN_BACKEND_ERROR
} stn_backend_status;

typedef struct stn_backend {
    stn_backend_type type;
    const char *name;
} stn_backend;

stn_backend_status stn_backend_select(
    stn_backend *backend,
    const stn_compute_inventory *compute_inventory
);

stn_backend_status stn_backend_search(
    const stn_backend *backend,
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
);

stn_backend_type stn_backend_candidate_type(
    const stn_gpu_inventory *inventory
);

const char *stn_backend_type_name(
    stn_backend_type type
);

#endif
