#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>

#include "stn_compute.h"
#include "stn_compute_platform.h"

static void stn_compute_win32_probe(
    stn_compute_inventory *inventory,
    stn_compute_provider_type type,
    const char *runtime
)
{
    HMODULE module;
    stn_compute_provider *provider;

    if (inventory == NULL ||
        runtime == NULL ||
        inventory->count >=
            STN_COMPUTE_MAX_PROVIDERS) {
        return;
    }

    module = LoadLibraryA(
        runtime
    );

    if (module == NULL) {
        return;
    }

    FreeLibrary(
        module
    );

    provider =
        &inventory->providers[
            inventory->count
        ];

    provider->type = type;
    provider->runtime = runtime;

    inventory->count++;
}

stn_compute_status stn_compute_platform_detect(
    stn_compute_inventory *inventory
)
{
    if (inventory == NULL) {
        return STN_COMPUTE_INVALID_ARGUMENT;
    }

    inventory->count = 0u;

    /*
     * Discovery only.
     *
     * Loading and immediately releasing a runtime confirms
     * presence without enumerating devices, creating a
     * compute context, compiling kernels, or hashing.
     */
    stn_compute_win32_probe(
        inventory,
        STN_COMPUTE_PROVIDER_OPENCL,
        "OpenCL.dll"
    );

    stn_compute_win32_probe(
        inventory,
        STN_COMPUTE_PROVIDER_CUDA,
        "nvcuda.dll"
    );

    stn_compute_win32_probe(
        inventory,
        STN_COMPUTE_PROVIDER_HIP,
        "amdhip64.dll"
    );

    stn_compute_win32_probe(
        inventory,
        STN_COMPUTE_PROVIDER_LEVEL_ZERO,
        "ze_loader.dll"
    );

    stn_compute_win32_probe(
        inventory,
        STN_COMPUTE_PROVIDER_VULKAN,
        "vulkan-1.dll"
    );

    stn_compute_win32_probe(
        inventory,
        STN_COMPUTE_PROVIDER_D3D12,
        "d3d12.dll"
    );

    return STN_COMPUTE_OK;
}
