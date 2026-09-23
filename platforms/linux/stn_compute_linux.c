#include <dlfcn.h>
#include <stddef.h>

#include "stn_compute.h"
#include "stn_compute_platform.h"

static void stn_compute_linux_probe(
    stn_compute_inventory *inventory,
    stn_compute_provider_type type,
    const char *runtime
)
{
    void *module;
    stn_compute_provider *provider;

    if (inventory == NULL ||
        runtime == NULL ||
        inventory->count >=
            STN_COMPUTE_MAX_PROVIDERS) {
        return;
    }

    module = dlopen(
        runtime,
        RTLD_LAZY | RTLD_LOCAL
    );

    if (module == NULL) {
        return;
    }

    dlclose(
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

    stn_compute_linux_probe(
        inventory,
        STN_COMPUTE_PROVIDER_OPENCL,
        "libOpenCL.so.1"
    );

    stn_compute_linux_probe(
        inventory,
        STN_COMPUTE_PROVIDER_CUDA,
        "libcuda.so.1"
    );

    stn_compute_linux_probe(
        inventory,
        STN_COMPUTE_PROVIDER_HIP,
        "libamdhip64.so"
    );

    stn_compute_linux_probe(
        inventory,
        STN_COMPUTE_PROVIDER_LEVEL_ZERO,
        "libze_loader.so.1"
    );

    stn_compute_linux_probe(
        inventory,
        STN_COMPUTE_PROVIDER_VULKAN,
        "libvulkan.so.1"
    );

    return STN_COMPUTE_OK;
}
