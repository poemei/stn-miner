#include "stn_gpu.h"
#include "stn_gpu_platform.h"

stn_gpu_status stn_gpu_detect(
    stn_gpu_inventory *inventory
)
{
    if (inventory == NULL) {
        return STN_GPU_INVALID_ARGUMENT;
    }

    inventory->count = 0u;

    return stn_gpu_platform_detect(
        inventory
    );
}

const char *stn_gpu_vendor_name(
    stn_gpu_vendor vendor
)
{
    switch (vendor) {
        case STN_GPU_VENDOR_NVIDIA:
            return "NVIDIA";

        case STN_GPU_VENDOR_AMD:
            return "AMD";

        case STN_GPU_VENDOR_INTEL:
            return "Intel";

        case STN_GPU_VENDOR_UNKNOWN:
        default:
            return "Unknown";
    }
}