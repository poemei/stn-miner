#include <stdio.h>
#include <string.h>

#include "stn_gpu_backend.h"
#include "stn_gpu_backend_platform.h"
#include "stn_protocol.h"

static int failures = 0;
static int platform_prepared = 0;

static void check(
    int condition,
    const char *name
)
{
    if (!condition) {
        ++failures;
        printf("FAIL: %s\n", name);
    }
}

stn_gpu_backend_status stn_gpu_backend_platform_prepare(
    const char *kernel_source
)
{
    if (kernel_source == NULL ||
        kernel_source[0] == '\0') {
        return STN_GPU_BACKEND_INVALID_ARGUMENT;
    }

    platform_prepared = 1;
    return STN_GPU_BACKEND_OK;
}

stn_gpu_backend_status stn_gpu_backend_platform_search(
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
)
{
    (void) job;
    (void) nonce_end;

    if (!platform_prepared ||
        solution == NULL) {
        return STN_GPU_BACKEND_UNAVAILABLE;
    }

    solution->nonce = nonce_start;
    return STN_GPU_BACKEND_NO_SOLUTION;
}

int main(void)
{
    stn_compute_inventory inventory;
    stn_miner_job job;
    stn_miner_solution solution;
    uint8_t block[STNM_BLOCK_HEADER_LENGTH];

    memset(&inventory, 0, sizeof(inventory));
    memset(&job, 0, sizeof(job));
    memset(&solution, 0, sizeof(solution));
    memset(block, 0, sizeof(block));

    check(
        stn_gpu_backend_available(NULL) == 0,
        "null inventory unavailable"
    );

    inventory.count = 1u;
    inventory.providers[0].type =
        STN_COMPUTE_PROVIDER_OPENCL;
    inventory.providers[0].sha256_ready = 1;

    check(
        stn_gpu_backend_available(&inventory) == 1,
        "qualified OpenCL available"
    );

    check(
        stn_gpu_backend_prepare() ==
            STN_GPU_BACKEND_OK,
        "prepare"
    );

    job.block = block;
    job.block_length =
        STNM_BLOCK_HEADER_LENGTH;

    check(
        stn_gpu_backend_search(
            &job,
            0u,
            4095u,
            &solution
        ) == STN_GPU_BACKEND_NO_SOLUTION,
        "bounded search delegated"
    );

    check(
        stn_gpu_backend_search(
            &job,
            0u,
            4096u,
            &solution
        ) == STN_GPU_BACKEND_INVALID_ARGUMENT,
        "oversized chunk rejected"
    );

    if (failures != 0) {
        printf(
            "GPU backend: %d failure(s).\n",
            failures
        );
        return 1;
    }

    printf("GPU backend: all checks passed.\n");
    return 0;
}
