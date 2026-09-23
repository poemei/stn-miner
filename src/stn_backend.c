#include "stn_backend.h"
#include "stn_cpu.h"

stn_backend_status stn_backend_select(
    stn_backend *backend
)
{
    if (backend == NULL) {
        return STN_BACKEND_INVALID_ARGUMENT;
    }

    backend->type =
        STN_BACKEND_TYPE_CPU;

    backend->name =
        "CPU";

    return STN_BACKEND_OK;
}

stn_backend_status stn_backend_search(
    const stn_backend *backend,
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
)
{
    stn_cpu_status cpu_status;

    if (backend == NULL ||
        job == NULL ||
        solution == NULL) {
        return STN_BACKEND_INVALID_ARGUMENT;
    }

    switch (backend->type) {
        case STN_BACKEND_TYPE_CPU:
            cpu_status =
                stn_cpu_search(
                    job,
                    nonce_start,
                    nonce_end,
                    solution
                );

            if (cpu_status ==
                STN_CPU_OK) {
                return STN_BACKEND_OK;
            }

            if (cpu_status ==
                STN_CPU_NO_SOLUTION) {
                return STN_BACKEND_NO_SOLUTION;
            }

            return STN_BACKEND_ERROR;

        case STN_BACKEND_TYPE_GPU:
        case STN_BACKEND_TYPE_USB_ASIC:
        case STN_BACKEND_TYPE_ASIC:
        case STN_BACKEND_TYPE_NONE:
        default:
            return STN_BACKEND_UNAVAILABLE;
    }
}

const char *stn_backend_type_name(
    stn_backend_type type
)
{
    switch (type) {
        case STN_BACKEND_TYPE_CPU:
            return "CPU";

        case STN_BACKEND_TYPE_GPU:
            return "GPU";

        case STN_BACKEND_TYPE_USB_ASIC:
            return "USB-ASIC";

        case STN_BACKEND_TYPE_ASIC:
            return "ASIC";

        case STN_BACKEND_TYPE_NONE:
        default:
            return "None";
    }
}
