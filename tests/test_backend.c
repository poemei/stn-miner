#include <stdio.h>
#include <string.h>

#include "stn_backend.h"
#include "stn_gpu_backend.h"

static int failures = 0;
static int gpu_prepare_ok = 0;

int stn_gpu_backend_available(
    const stn_compute_inventory *inventory
)
{
    if (inventory == NULL ||
        inventory->count == 0u) {
        return 0;
    }

    return
        inventory->providers[0].type ==
            STN_COMPUTE_PROVIDER_OPENCL &&
        inventory->providers[0].sha256_ready;
}

stn_gpu_backend_status stn_gpu_backend_prepare(void)
{
    return gpu_prepare_ok
        ? STN_GPU_BACKEND_OK
        : STN_GPU_BACKEND_UNAVAILABLE;
}

stn_gpu_backend_status stn_gpu_backend_search(
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
)
{
    (void) job;
    (void) nonce_start;
    (void) nonce_end;
    (void) solution;

    return STN_GPU_BACKEND_NO_SOLUTION;
}

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

int main(void)
{
    stn_backend backend;
    stn_gpu_inventory gpu_inventory;
    stn_compute_inventory compute_inventory;

    memset(
        &backend,
        0,
        sizeof(backend)
    );

    memset(
        &gpu_inventory,
        0,
        sizeof(gpu_inventory)
    );

    memset(
        &compute_inventory,
        0,
        sizeof(compute_inventory)
    );

    check(
        stn_backend_select(
            NULL,
            &compute_inventory
        ) == STN_BACKEND_INVALID_ARGUMENT,
        "null backend rejected"
    );

    check(
        stn_backend_select(
            &backend,
            &compute_inventory
        ) == STN_BACKEND_OK,
        "CPU backend select"
    );

    check(
        backend.type ==
            STN_BACKEND_TYPE_CPU,
        "CPU selected without qualified GPU"
    );

    compute_inventory.count = 1u;
    compute_inventory.providers[0].type =
        STN_COMPUTE_PROVIDER_OPENCL;
    compute_inventory.providers[0].sha256_ready = 1;
    gpu_prepare_ok = 1;

    check(
        stn_backend_select(
            &backend,
            &compute_inventory
        ) == STN_BACKEND_OK,
        "GPU backend select"
    );

    check(
        backend.type ==
            STN_BACKEND_TYPE_GPU,
        "GPU selected when qualified"
    );

    check(
        backend.name != NULL &&
        strcmp(
            backend.name,
            "GPU"
        ) == 0,
        "GPU name"
    );

    check(
        stn_backend_candidate_type(
            &gpu_inventory
        ) == STN_BACKEND_TYPE_CPU,
        "CPU candidate without GPU"
    );

    gpu_inventory.count = 1u;

    check(
        stn_backend_candidate_type(
            &gpu_inventory
        ) == STN_BACKEND_TYPE_GPU,
        "GPU candidate when detected"
    );

    if (failures != 0) {
        printf(
            "Backend: %d failure(s).\n",
            failures
        );
        return 1;
    }

    printf("Backend: all checks passed.\n");
    return 0;
}
