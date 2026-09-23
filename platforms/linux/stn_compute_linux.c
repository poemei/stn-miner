#include <dlfcn.h>
#include <stddef.h>

#include "stn_compute.h"
#include "stn_compute_platform.h"

static int stn_compute_linux_has_symbol(
    void *module,
    const char *name
)
{
    if (module == NULL ||
        name == NULL) {
        return 0;
    }

    return dlsym(
        module,
        name
    ) != NULL;
}

static int stn_compute_linux_api_ready(
    void *module,
    stn_compute_provider_type type
)
{
    switch (type) {
        case STN_COMPUTE_PROVIDER_OPENCL:
            return
                stn_compute_linux_has_symbol(module, "clGetPlatformIDs") &&
                stn_compute_linux_has_symbol(module, "clGetDeviceIDs") &&
                stn_compute_linux_has_symbol(module, "clGetDeviceInfo") &&
                stn_compute_linux_has_symbol(module, "clCreateContext") &&
                stn_compute_linux_has_symbol(module, "clCreateCommandQueue") &&
                stn_compute_linux_has_symbol(module, "clCreateProgramWithSource") &&
                stn_compute_linux_has_symbol(module, "clBuildProgram") &&
                stn_compute_linux_has_symbol(module, "clCreateKernel") &&
                stn_compute_linux_has_symbol(module, "clCreateBuffer") &&
                stn_compute_linux_has_symbol(module, "clSetKernelArg") &&
                stn_compute_linux_has_symbol(module, "clEnqueueNDRangeKernel") &&
                stn_compute_linux_has_symbol(module, "clEnqueueReadBuffer") &&
                stn_compute_linux_has_symbol(module, "clFinish");

        case STN_COMPUTE_PROVIDER_CUDA:
            return
                stn_compute_linux_has_symbol(module, "cuInit") &&
                stn_compute_linux_has_symbol(module, "cuDeviceGetCount") &&
                stn_compute_linux_has_symbol(module, "cuDeviceGet") &&
                stn_compute_linux_has_symbol(module, "cuCtxCreate_v2") &&
                stn_compute_linux_has_symbol(module, "cuModuleLoadDataEx") &&
                stn_compute_linux_has_symbol(module, "cuModuleGetFunction") &&
                stn_compute_linux_has_symbol(module, "cuLaunchKernel");

        case STN_COMPUTE_PROVIDER_HIP:
            return
                stn_compute_linux_has_symbol(module, "hipInit") &&
                stn_compute_linux_has_symbol(module, "hipGetDeviceCount") &&
                stn_compute_linux_has_symbol(module, "hipSetDevice") &&
                stn_compute_linux_has_symbol(module, "hipModuleLoadData") &&
                stn_compute_linux_has_symbol(module, "hipModuleGetFunction") &&
                stn_compute_linux_has_symbol(module, "hipModuleLaunchKernel");

        case STN_COMPUTE_PROVIDER_LEVEL_ZERO:
            return
                stn_compute_linux_has_symbol(module, "zeInit") &&
                stn_compute_linux_has_symbol(module, "zeDriverGet") &&
                stn_compute_linux_has_symbol(module, "zeDeviceGet") &&
                stn_compute_linux_has_symbol(module, "zeContextCreate") &&
                stn_compute_linux_has_symbol(module, "zeModuleCreate") &&
                stn_compute_linux_has_symbol(module, "zeKernelCreate");

        case STN_COMPUTE_PROVIDER_VULKAN:
            return
                stn_compute_linux_has_symbol(module, "vkGetInstanceProcAddr") &&
                stn_compute_linux_has_symbol(module, "vkCreateInstance") &&
                stn_compute_linux_has_symbol(module, "vkEnumeratePhysicalDevices");

        case STN_COMPUTE_PROVIDER_D3D12:
        default:
            return 0;
    }
}

typedef int (*stn_opencl_get_platform_ids_fn)(
    unsigned int,
    void **,
    unsigned int *
);

static void stn_compute_linux_qualify_opencl_platforms(
    void *module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    unsigned int platform_count;
    int result;

    if (module == NULL ||
        provider == NULL ||
        !provider->api_ready) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(
            module,
            "clGetPlatformIDs"
        );

    if (get_platform_ids == NULL) {
        return;
    }

    platform_count = 0u;

    result =
        get_platform_ids(
            0u,
            NULL,
            &platform_count
        );

    if (result != 0) {
        return;
    }

    provider->platform_count =
        (size_t) platform_count;

    provider->platform_ready =
        platform_count > 0u ? 1 : 0;
}

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

    provider =
        &inventory->providers[
            inventory->count
        ];

    provider->type = type;
    provider->runtime = runtime;
    provider->api_ready =
        stn_compute_linux_api_ready(
            module,
            type
        );

    provider->platform_ready = 0;
    provider->platform_count = 0u;

    if (type ==
        STN_COMPUTE_PROVIDER_OPENCL) {
        stn_compute_linux_qualify_opencl_platforms(
            module,
            provider
        );
    }

    inventory->count++;

    dlclose(
        module
    );
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
