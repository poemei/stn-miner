#include "stn_compute.h"
#include "stn_compute_platform.h"

stn_compute_status stn_compute_detect(
    stn_compute_inventory *inventory
)
{
    if (inventory == NULL) {
        return STN_COMPUTE_INVALID_ARGUMENT;
    }

    inventory->count = 0u;

    return stn_compute_platform_detect(
        inventory
    );
}

const char *stn_compute_provider_name(
    stn_compute_provider_type type
)
{
    switch (type) {
        case STN_COMPUTE_PROVIDER_OPENCL:
            return "OpenCL";

        case STN_COMPUTE_PROVIDER_CUDA:
            return "CUDA";

        case STN_COMPUTE_PROVIDER_HIP:
            return "HIP";

        case STN_COMPUTE_PROVIDER_LEVEL_ZERO:
            return "Level Zero";

        case STN_COMPUTE_PROVIDER_VULKAN:
            return "Vulkan";

        case STN_COMPUTE_PROVIDER_D3D12:
            return "Direct3D 12";

        default:
            return "Unknown";
    }
}
