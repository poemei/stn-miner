#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>

#include "stn_compute.h"
#include "stn_compute_platform.h"

static int stn_compute_win32_has_symbol(
    HMODULE module,
    const char *name
)
{
    if (module == NULL ||
        name == NULL) {
        return 0;
    }

    return GetProcAddress(
        module,
        name
    ) != NULL;
}

static int stn_compute_win32_api_ready(
    HMODULE module,
    stn_compute_provider_type type
)
{
    switch (type) {
        case STN_COMPUTE_PROVIDER_OPENCL:
            return
                stn_compute_win32_has_symbol(module, "clGetPlatformIDs") &&
                stn_compute_win32_has_symbol(module, "clGetDeviceIDs") &&
                stn_compute_win32_has_symbol(module, "clGetDeviceInfo") &&
                stn_compute_win32_has_symbol(module, "clCreateContext") &&
                stn_compute_win32_has_symbol(module, "clCreateCommandQueue") &&
                stn_compute_win32_has_symbol(module, "clCreateProgramWithSource") &&
                stn_compute_win32_has_symbol(module, "clBuildProgram") &&
                stn_compute_win32_has_symbol(module, "clCreateKernel") &&
                stn_compute_win32_has_symbol(module, "clCreateBuffer") &&
                stn_compute_win32_has_symbol(module, "clSetKernelArg") &&
                stn_compute_win32_has_symbol(module, "clEnqueueNDRangeKernel") &&
                stn_compute_win32_has_symbol(module, "clEnqueueReadBuffer") &&
                stn_compute_win32_has_symbol(module, "clFinish");

        case STN_COMPUTE_PROVIDER_CUDA:
            return
                stn_compute_win32_has_symbol(module, "cuInit") &&
                stn_compute_win32_has_symbol(module, "cuDeviceGetCount") &&
                stn_compute_win32_has_symbol(module, "cuDeviceGet") &&
                stn_compute_win32_has_symbol(module, "cuCtxCreate_v2") &&
                stn_compute_win32_has_symbol(module, "cuModuleLoadDataEx") &&
                stn_compute_win32_has_symbol(module, "cuModuleGetFunction") &&
                stn_compute_win32_has_symbol(module, "cuLaunchKernel");

        case STN_COMPUTE_PROVIDER_HIP:
            return
                stn_compute_win32_has_symbol(module, "hipInit") &&
                stn_compute_win32_has_symbol(module, "hipGetDeviceCount") &&
                stn_compute_win32_has_symbol(module, "hipSetDevice") &&
                stn_compute_win32_has_symbol(module, "hipModuleLoadData") &&
                stn_compute_win32_has_symbol(module, "hipModuleGetFunction") &&
                stn_compute_win32_has_symbol(module, "hipModuleLaunchKernel");

        case STN_COMPUTE_PROVIDER_LEVEL_ZERO:
            return
                stn_compute_win32_has_symbol(module, "zeInit") &&
                stn_compute_win32_has_symbol(module, "zeDriverGet") &&
                stn_compute_win32_has_symbol(module, "zeDeviceGet") &&
                stn_compute_win32_has_symbol(module, "zeContextCreate") &&
                stn_compute_win32_has_symbol(module, "zeModuleCreate") &&
                stn_compute_win32_has_symbol(module, "zeKernelCreate");

        case STN_COMPUTE_PROVIDER_VULKAN:
            return
                stn_compute_win32_has_symbol(module, "vkGetInstanceProcAddr") &&
                stn_compute_win32_has_symbol(module, "vkCreateInstance") &&
                stn_compute_win32_has_symbol(module, "vkEnumeratePhysicalDevices");

        case STN_COMPUTE_PROVIDER_D3D12:
            return
                stn_compute_win32_has_symbol(module, "D3D12CreateDevice") &&
                stn_compute_win32_has_symbol(module, "D3D12SerializeRootSignature");

        default:
            return 0;
    }
}

typedef int (__stdcall *stn_opencl_get_platform_ids_fn)(
    unsigned int,
    void **,
    unsigned int *
);

typedef int (__stdcall *stn_opencl_get_device_ids_fn)(
    void *,
    unsigned long long,
    unsigned int,
    void **,
    unsigned int *
);

#define STN_OPENCL_DEVICE_TYPE_GPU (1ull << 2)
#define STN_OPENCL_DEVICE_NOT_FOUND (-1)
#define STN_OPENCL_MAX_PLATFORMS 16u

static void stn_compute_win32_qualify_opencl_platforms(
    HMODULE module,
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
        GetProcAddress(
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


static void stn_compute_win32_qualify_opencl_devices(
    HMODULE module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    unsigned int platform_count;
    unsigned int gpu_count;
    size_t total_gpu_count;
    unsigned int i;
    int result;

    if (module == NULL ||
        provider == NULL ||
        !provider->platform_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        GetProcAddress(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        GetProcAddress(module, "clGetDeviceIDs");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL) {
        return;
    }

    platform_count =
        (unsigned int)
        provider->platform_count;

    result =
        get_platform_ids(
            platform_count,
            platforms,
            NULL
        );

    if (result != 0) {
        return;
    }

    total_gpu_count = 0u;

    for (i = 0u;
         i < platform_count;
         ++i) {

        gpu_count = 0u;

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                0u,
                NULL,
                &gpu_count
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0) {
            return;
        }

        total_gpu_count +=
            (size_t) gpu_count;
    }

    provider->device_query_ready = 1;
    provider->device_count =
        total_gpu_count;
}

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

    provider =
        &inventory->providers[
            inventory->count
        ];

    provider->type = type;
    provider->runtime = runtime;
    provider->api_ready =
        stn_compute_win32_api_ready(
            module,
            type
        );

    provider->platform_ready = 0;
    provider->platform_count = 0u;
    provider->device_query_ready = 0;
    provider->device_count = 0u;

    if (type ==
        STN_COMPUTE_PROVIDER_OPENCL) {
        stn_compute_win32_qualify_opencl_platforms(
            module,
            provider
        );

        stn_compute_win32_qualify_opencl_devices(
            module,
            provider
        );
    }

    inventory->count++;

    FreeLibrary(
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

    /*
     * Runtime/API qualification only.
     *
     * This stage loads each runtime and checks the symbols
     * required for a future compute backend. It does not
     * enumerate devices, create contexts, compile kernels,
     * or execute work. OpenCL alone is allowed to query
     * platform count at this stage.
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
