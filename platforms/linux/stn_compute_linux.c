#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
                stn_compute_linux_has_symbol(module, "clReleaseContext") &&
                stn_compute_linux_has_symbol(module, "clCreateCommandQueue") &&
                stn_compute_linux_has_symbol(module, "clReleaseCommandQueue") &&
                stn_compute_linux_has_symbol(module, "clCreateProgramWithSource") &&
                stn_compute_linux_has_symbol(module, "clReleaseProgram") &&
                stn_compute_linux_has_symbol(module, "clBuildProgram") &&
                stn_compute_linux_has_symbol(module, "clCreateKernel") &&
                stn_compute_linux_has_symbol(module, "clReleaseKernel") &&
                stn_compute_linux_has_symbol(module, "clCreateBuffer") &&
                stn_compute_linux_has_symbol(module, "clReleaseMemObject") &&
                stn_compute_linux_has_symbol(module, "clSetKernelArg") &&
                stn_compute_linux_has_symbol(module, "clEnqueueNDRangeKernel") &&
                stn_compute_linux_has_symbol(module, "clEnqueueWriteBuffer") &&
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

typedef int (*stn_opencl_get_device_ids_fn)(
    void *,
    unsigned long long,
    unsigned int,
    void **,
    unsigned int *
);

typedef int (*stn_opencl_get_device_info_fn)(
    void *,
    unsigned int,
    size_t,
    void *,
    size_t *
);

typedef void *(*stn_opencl_create_context_fn)(
    const intptr_t *,
    unsigned int,
    void * const *,
    void *,
    void *,
    int *
);

typedef int (*stn_opencl_release_context_fn)(
    void *
);

typedef void *(*stn_opencl_create_command_queue_fn)(
    void *,
    void *,
    unsigned long long,
    int *
);

typedef int (*stn_opencl_release_command_queue_fn)(
    void *
);

typedef void *(*stn_opencl_create_buffer_fn)(
    void *,
    unsigned long long,
    size_t,
    void *,
    int *
);

typedef int (*stn_opencl_release_mem_object_fn)(
    void *
);

typedef int (*stn_opencl_enqueue_write_buffer_fn)(
    void *,
    void *,
    unsigned int,
    size_t,
    size_t,
    const void *,
    unsigned int,
    const void *,
    void *
);

typedef int (*stn_opencl_enqueue_read_buffer_fn)(
    void *,
    void *,
    unsigned int,
    size_t,
    size_t,
    void *,
    unsigned int,
    const void *,
    void *
);

typedef void *(*stn_opencl_create_program_with_source_fn)(
    void *,
    unsigned int,
    const char **,
    const size_t *,
    int *
);

typedef int (*stn_opencl_release_program_fn)(
    void *
);

typedef int (*stn_opencl_build_program_fn)(
    void *,
    unsigned int,
    void * const *,
    const char *,
    void *,
    void *
);

typedef void *(*stn_opencl_create_kernel_fn)(
    void *,
    const char *,
    int *
);

typedef int (*stn_opencl_release_kernel_fn)(
    void *
);

typedef int (*stn_opencl_set_kernel_arg_fn)(
    void *,
    unsigned int,
    size_t,
    const void *
);

typedef int (*stn_opencl_enqueue_ndrange_kernel_fn)(
    void *,
    void *,
    unsigned int,
    const size_t *,
    const size_t *,
    const size_t *,
    unsigned int,
    const void *,
    void *
);

typedef int (*stn_opencl_finish_fn)(
    void *
);

#define STN_OPENCL_DEVICE_TYPE_GPU (1ull << 2)
#define STN_OPENCL_DEVICE_NOT_FOUND (-1)
#define STN_OPENCL_MAX_PLATFORMS 16u
#define STN_OPENCL_DEVICE_NAME 0x102bu
#define STN_OPENCL_DEVICE_VENDOR 0x102cu
#define STN_OPENCL_MEM_READ_WRITE (1ull << 0)
#define STN_OPENCL_QUALIFY_BUFFER_SIZE 64u
#define STN_OPENCL_TRUE 1u

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


static void stn_compute_linux_qualify_opencl_devices(
    void * module,
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
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

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

static void stn_compute_linux_qualify_opencl_identity(
    void * module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_get_device_info_fn get_device_info;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    unsigned int platform_count;
    unsigned int i;
    int result;

    if (module == NULL ||
        provider == NULL ||
        !provider->device_query_ready ||
        provider->device_count == 0u ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    get_device_info =
        (stn_opencl_get_device_info_fn)
        dlsym(module, "clGetDeviceInfo");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        get_device_info == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result =
        get_device_info(
            device,
            STN_OPENCL_DEVICE_NAME,
            sizeof(provider->device_name),
            provider->device_name,
            NULL
        );

    if (result != 0) {
        return;
    }

    provider->device_name[
        sizeof(provider->device_name) - 1u
    ] = '\0';

    result =
        get_device_info(
            device,
            STN_OPENCL_DEVICE_VENDOR,
            sizeof(provider->device_vendor),
            provider->device_vendor,
            NULL
        );

    if (result != 0) {
        provider->device_vendor[0] = '\0';
        return;
    }

    provider->device_vendor[
        sizeof(provider->device_vendor) - 1u
    ] = '\0';

    provider->device_identity_ready = 1;
}

static void stn_compute_linux_qualify_opencl_context(
    void * module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->device_identity_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    release_result =
        release_context(
            context
        );

    if (release_result != 0) {
        return;
    }

    provider->context_ready = 1;
}

static void stn_compute_linux_qualify_opencl_queue(
    void * module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_command_queue_fn create_queue;
    stn_opencl_release_command_queue_fn release_queue;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *queue;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int queue_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->context_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_queue =
        (stn_opencl_create_command_queue_fn)
        dlsym(module, "clCreateCommandQueue");

    release_queue =
        (stn_opencl_release_command_queue_fn)
        dlsym(module, "clReleaseCommandQueue");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_queue == NULL ||
        release_queue == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    result = 0;

    queue =
        create_queue(
            context,
            device,
            0u,
            &result
        );

    if (queue == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    queue_release_result =
        release_queue(
            queue
        );

    context_release_result =
        release_context(
            context
        );

    if (queue_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->queue_ready = 1;
}

static void stn_compute_linux_qualify_opencl_buffer(
    void * module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_command_queue_fn create_queue;
    stn_opencl_release_command_queue_fn release_queue;
    stn_opencl_create_buffer_fn create_buffer;
    stn_opencl_release_mem_object_fn release_mem_object;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *queue;
    void *buffer;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int buffer_release_result;
    int queue_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->queue_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_queue =
        (stn_opencl_create_command_queue_fn)
        dlsym(module, "clCreateCommandQueue");

    release_queue =
        (stn_opencl_release_command_queue_fn)
        dlsym(module, "clReleaseCommandQueue");

    create_buffer =
        (stn_opencl_create_buffer_fn)
        dlsym(module, "clCreateBuffer");

    release_mem_object =
        (stn_opencl_release_mem_object_fn)
        dlsym(module, "clReleaseMemObject");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_queue == NULL ||
        release_queue == NULL ||
        create_buffer == NULL ||
        release_mem_object == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    result = 0;

    queue =
        create_queue(
            context,
            device,
            0u,
            &result
        );

    if (queue == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    buffer =
        create_buffer(
            context,
            STN_OPENCL_MEM_READ_WRITE,
            STN_OPENCL_QUALIFY_BUFFER_SIZE,
            NULL,
            &result
        );

    if (buffer == NULL ||
        result != 0) {

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    buffer_release_result =
        release_mem_object(
            buffer
        );

    queue_release_result =
        release_queue(
            queue
        );

    context_release_result =
        release_context(
            context
        );

    if (buffer_release_result != 0 ||
        queue_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->buffer_ready = 1;
}

static void stn_compute_linux_qualify_opencl_transfer(
    void * module,
    stn_compute_provider *provider
)
{
    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_command_queue_fn create_queue;
    stn_opencl_release_command_queue_fn release_queue;
    stn_opencl_create_buffer_fn create_buffer;
    stn_opencl_release_mem_object_fn release_mem_object;
    stn_opencl_enqueue_write_buffer_fn enqueue_write_buffer;
    stn_opencl_enqueue_read_buffer_fn enqueue_read_buffer;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *queue;
    void *buffer;
    unsigned char source[16];
    unsigned char destination[16];
    unsigned int platform_count;
    unsigned int i;
    int result;
    int buffer_release_result;
    int queue_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->buffer_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_queue =
        (stn_opencl_create_command_queue_fn)
        dlsym(module, "clCreateCommandQueue");

    release_queue =
        (stn_opencl_release_command_queue_fn)
        dlsym(module, "clReleaseCommandQueue");

    create_buffer =
        (stn_opencl_create_buffer_fn)
        dlsym(module, "clCreateBuffer");

    release_mem_object =
        (stn_opencl_release_mem_object_fn)
        dlsym(module, "clReleaseMemObject");

    enqueue_write_buffer =
        (stn_opencl_enqueue_write_buffer_fn)
        dlsym(module, "clEnqueueWriteBuffer");

    enqueue_read_buffer =
        (stn_opencl_enqueue_read_buffer_fn)
        dlsym(module, "clEnqueueReadBuffer");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_queue == NULL ||
        release_queue == NULL ||
        create_buffer == NULL ||
        release_mem_object == NULL ||
        enqueue_write_buffer == NULL ||
        enqueue_read_buffer == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    result = 0;

    queue =
        create_queue(
            context,
            device,
            0u,
            &result
        );

    if (queue == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    buffer =
        create_buffer(
            context,
            STN_OPENCL_MEM_READ_WRITE,
            sizeof(source),
            NULL,
            &result
        );

    if (buffer == NULL ||
        result != 0) {

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    for (i = 0u;
         i < (unsigned int) sizeof(source);
         ++i) {

        source[i] =
            (unsigned char)
            ((i * 17u) + 3u);
    }

    memset(
        destination,
        0,
        sizeof(destination)
    );

    result =
        enqueue_write_buffer(
            queue,
            buffer,
            STN_OPENCL_TRUE,
            0u,
            sizeof(source),
            source,
            0u,
            NULL,
            NULL
        );

    if (result == 0) {
        result =
            enqueue_read_buffer(
                queue,
                buffer,
                STN_OPENCL_TRUE,
                0u,
                sizeof(destination),
                destination,
                0u,
                NULL,
                NULL
            );
    }

    buffer_release_result =
        release_mem_object(
            buffer
        );

    queue_release_result =
        release_queue(
            queue
        );

    context_release_result =
        release_context(
            context
        );

    if (result != 0 ||
        buffer_release_result != 0 ||
        queue_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    if (memcmp(
            source,
            destination,
            sizeof(source)
        ) != 0) {
        return;
    }

    provider->transfer_ready = 1;
}

static void stn_compute_linux_qualify_opencl_program(
    void * module,
    stn_compute_provider *provider
)
{
    static const char source_text[] =
        "__kernel void stn_qualify(void) { }";

    const char *source;

    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_program_with_source_fn create_program;
    stn_opencl_release_program_fn release_program;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *program;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int program_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->transfer_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_program =
        (stn_opencl_create_program_with_source_fn)
        dlsym(module, "clCreateProgramWithSource");

    release_program =
        (stn_opencl_release_program_fn)
        dlsym(module, "clReleaseProgram");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_program == NULL ||
        release_program == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    source = source_text;
    result = 0;

    program =
        create_program(
            context,
            1u,
            &source,
            NULL,
            &result
        );

    if (program == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    program_release_result =
        release_program(
            program
        );

    context_release_result =
        release_context(
            context
        );

    if (program_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->program_ready = 1;
}

static void stn_compute_linux_qualify_opencl_build(
    void * module,
    stn_compute_provider *provider
)
{
    static const char source_text[] =
        "__kernel void stn_qualify(void) { }";

    const char *source;

    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_program_with_source_fn create_program;
    stn_opencl_release_program_fn release_program;
    stn_opencl_build_program_fn build_program;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *program;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int build_result;
    int program_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->program_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_program =
        (stn_opencl_create_program_with_source_fn)
        dlsym(module, "clCreateProgramWithSource");

    release_program =
        (stn_opencl_release_program_fn)
        dlsym(module, "clReleaseProgram");

    build_program =
        (stn_opencl_build_program_fn)
        dlsym(module, "clBuildProgram");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_program == NULL ||
        release_program == NULL ||
        build_program == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    source = source_text;
    result = 0;

    program =
        create_program(
            context,
            1u,
            &source,
            NULL,
            &result
        );

    if (program == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    build_result =
        build_program(
            program,
            1u,
            &device,
            NULL,
            NULL,
            NULL
        );

    program_release_result =
        release_program(
            program
        );

    context_release_result =
        release_context(
            context
        );

    if (build_result != 0 ||
        program_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->build_ready = 1;
}

static void stn_compute_linux_qualify_opencl_kernel(
    void * module,
    stn_compute_provider *provider
)
{
    static const char source_text[] =
        "__kernel void stn_qualify(void) { }";

    const char *source;

    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_program_with_source_fn create_program;
    stn_opencl_release_program_fn release_program;
    stn_opencl_build_program_fn build_program;
    stn_opencl_create_kernel_fn create_kernel;
    stn_opencl_release_kernel_fn release_kernel;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *program;
    void *kernel;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int build_result;
    int kernel_release_result;
    int program_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->build_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_program =
        (stn_opencl_create_program_with_source_fn)
        dlsym(module, "clCreateProgramWithSource");

    release_program =
        (stn_opencl_release_program_fn)
        dlsym(module, "clReleaseProgram");

    build_program =
        (stn_opencl_build_program_fn)
        dlsym(module, "clBuildProgram");

    create_kernel =
        (stn_opencl_create_kernel_fn)
        dlsym(module, "clCreateKernel");

    release_kernel =
        (stn_opencl_release_kernel_fn)
        dlsym(module, "clReleaseKernel");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_program == NULL ||
        release_program == NULL ||
        build_program == NULL ||
        create_kernel == NULL ||
        release_kernel == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    source = source_text;
    result = 0;

    program =
        create_program(
            context,
            1u,
            &source,
            NULL,
            &result
        );

    if (program == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    build_result =
        build_program(
            program,
            1u,
            &device,
            NULL,
            NULL,
            NULL
        );

    if (build_result != 0) {

        (void) release_program(
            program
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    kernel =
        create_kernel(
            program,
            "stn_qualify",
            &result
        );

    if (kernel == NULL ||
        result != 0) {

        (void) release_program(
            program
        );

        (void) release_context(
            context
        );

        return;
    }

    kernel_release_result =
        release_kernel(
            kernel
        );

    program_release_result =
        release_program(
            program
        );

    context_release_result =
        release_context(
            context
        );

    if (kernel_release_result != 0 ||
        program_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->kernel_ready = 1;
}

static void stn_compute_linux_qualify_opencl_argument(
    void * module,
    stn_compute_provider *provider
)
{
    static const char source_text[] =
        "__kernel void stn_qualify(__global uchar *data) { (void)data; }";

    const char *source;

    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_program_with_source_fn create_program;
    stn_opencl_release_program_fn release_program;
    stn_opencl_build_program_fn build_program;
    stn_opencl_create_kernel_fn create_kernel;
    stn_opencl_release_kernel_fn release_kernel;
    stn_opencl_create_buffer_fn create_buffer;
    stn_opencl_release_mem_object_fn release_mem_object;
    stn_opencl_set_kernel_arg_fn set_kernel_arg;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *program;
    void *kernel;
    void *buffer;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int build_result;
    int argument_result;
    int buffer_release_result;
    int kernel_release_result;
    int program_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->kernel_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_program =
        (stn_opencl_create_program_with_source_fn)
        dlsym(module, "clCreateProgramWithSource");

    release_program =
        (stn_opencl_release_program_fn)
        dlsym(module, "clReleaseProgram");

    build_program =
        (stn_opencl_build_program_fn)
        dlsym(module, "clBuildProgram");

    create_kernel =
        (stn_opencl_create_kernel_fn)
        dlsym(module, "clCreateKernel");

    release_kernel =
        (stn_opencl_release_kernel_fn)
        dlsym(module, "clReleaseKernel");

    create_buffer =
        (stn_opencl_create_buffer_fn)
        dlsym(module, "clCreateBuffer");

    release_mem_object =
        (stn_opencl_release_mem_object_fn)
        dlsym(module, "clReleaseMemObject");

    set_kernel_arg =
        (stn_opencl_set_kernel_arg_fn)
        dlsym(module, "clSetKernelArg");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_program == NULL ||
        release_program == NULL ||
        build_program == NULL ||
        create_kernel == NULL ||
        release_kernel == NULL ||
        create_buffer == NULL ||
        release_mem_object == NULL ||
        set_kernel_arg == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    source = source_text;
    result = 0;

    program =
        create_program(
            context,
            1u,
            &source,
            NULL,
            &result
        );

    if (program == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    build_result =
        build_program(
            program,
            1u,
            &device,
            NULL,
            NULL,
            NULL
        );

    if (build_result != 0) {

        (void) release_program(
            program
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    kernel =
        create_kernel(
            program,
            "stn_qualify",
            &result
        );

    if (kernel == NULL ||
        result != 0) {

        (void) release_program(
            program
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    buffer =
        create_buffer(
            context,
            STN_OPENCL_MEM_READ_WRITE,
            STN_OPENCL_QUALIFY_BUFFER_SIZE,
            NULL,
            &result
        );

    if (buffer == NULL ||
        result != 0) {

        (void) release_kernel(
            kernel
        );

        (void) release_program(
            program
        );

        (void) release_context(
            context
        );

        return;
    }

    argument_result =
        set_kernel_arg(
            kernel,
            0u,
            sizeof(buffer),
            &buffer
        );

    buffer_release_result =
        release_mem_object(
            buffer
        );

    kernel_release_result =
        release_kernel(
            kernel
        );

    program_release_result =
        release_program(
            program
        );

    context_release_result =
        release_context(
            context
        );

    if (argument_result != 0 ||
        buffer_release_result != 0 ||
        kernel_release_result != 0 ||
        program_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->argument_ready = 1;
}

static void stn_compute_linux_qualify_opencl_execution(
    void * module,
    stn_compute_provider *provider
)
{
    static const char source_text[] =
        "__kernel void stn_qualify(__global uchar *data) { (void)data; }";

    const char *source;

    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_command_queue_fn create_queue;
    stn_opencl_release_command_queue_fn release_queue;
    stn_opencl_create_program_with_source_fn create_program;
    stn_opencl_release_program_fn release_program;
    stn_opencl_build_program_fn build_program;
    stn_opencl_create_kernel_fn create_kernel;
    stn_opencl_release_kernel_fn release_kernel;
    stn_opencl_create_buffer_fn create_buffer;
    stn_opencl_release_mem_object_fn release_mem_object;
    stn_opencl_set_kernel_arg_fn set_kernel_arg;
    stn_opencl_enqueue_ndrange_kernel_fn enqueue_ndrange_kernel;
    stn_opencl_finish_fn finish;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *queue;
    void *program;
    void *kernel;
    void *buffer;
    size_t global_work_size;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int build_result;
    int argument_result;
    int enqueue_result;
    int finish_result;
    int buffer_release_result;
    int kernel_release_result;
    int program_release_result;
    int queue_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->argument_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_queue =
        (stn_opencl_create_command_queue_fn)
        dlsym(module, "clCreateCommandQueue");

    release_queue =
        (stn_opencl_release_command_queue_fn)
        dlsym(module, "clReleaseCommandQueue");

    create_program =
        (stn_opencl_create_program_with_source_fn)
        dlsym(module, "clCreateProgramWithSource");

    release_program =
        (stn_opencl_release_program_fn)
        dlsym(module, "clReleaseProgram");

    build_program =
        (stn_opencl_build_program_fn)
        dlsym(module, "clBuildProgram");

    create_kernel =
        (stn_opencl_create_kernel_fn)
        dlsym(module, "clCreateKernel");

    release_kernel =
        (stn_opencl_release_kernel_fn)
        dlsym(module, "clReleaseKernel");

    create_buffer =
        (stn_opencl_create_buffer_fn)
        dlsym(module, "clCreateBuffer");

    release_mem_object =
        (stn_opencl_release_mem_object_fn)
        dlsym(module, "clReleaseMemObject");

    set_kernel_arg =
        (stn_opencl_set_kernel_arg_fn)
        dlsym(module, "clSetKernelArg");

    enqueue_ndrange_kernel =
        (stn_opencl_enqueue_ndrange_kernel_fn)
        dlsym(module, "clEnqueueNDRangeKernel");

    finish =
        (stn_opencl_finish_fn)
        dlsym(module, "clFinish");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_queue == NULL ||
        release_queue == NULL ||
        create_program == NULL ||
        release_program == NULL ||
        build_program == NULL ||
        create_kernel == NULL ||
        release_kernel == NULL ||
        create_buffer == NULL ||
        release_mem_object == NULL ||
        set_kernel_arg == NULL ||
        enqueue_ndrange_kernel == NULL ||
        finish == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    result = 0;

    queue =
        create_queue(
            context,
            device,
            0u,
            &result
        );

    if (queue == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    source = source_text;
    result = 0;

    program =
        create_program(
            context,
            1u,
            &source,
            NULL,
            &result
        );

    if (program == NULL ||
        result != 0) {

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    build_result =
        build_program(
            program,
            1u,
            &device,
            NULL,
            NULL,
            NULL
        );

    if (build_result != 0) {

        (void) release_program(
            program
        );

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    kernel =
        create_kernel(
            program,
            "stn_qualify",
            &result
        );

    if (kernel == NULL ||
        result != 0) {

        (void) release_program(
            program
        );

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    buffer =
        create_buffer(
            context,
            STN_OPENCL_MEM_READ_WRITE,
            STN_OPENCL_QUALIFY_BUFFER_SIZE,
            NULL,
            &result
        );

    if (buffer == NULL ||
        result != 0) {

        (void) release_kernel(
            kernel
        );

        (void) release_program(
            program
        );

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    argument_result =
        set_kernel_arg(
            kernel,
            0u,
            sizeof(buffer),
            &buffer
        );

    global_work_size = 1u;
    enqueue_result = -1;
    finish_result = -1;

    if (argument_result == 0) {
        enqueue_result =
            enqueue_ndrange_kernel(
                queue,
                kernel,
                1u,
                NULL,
                &global_work_size,
                NULL,
                0u,
                NULL,
                NULL
            );
    }

    if (enqueue_result == 0) {
        finish_result =
            finish(
                queue
            );
    }

    buffer_release_result =
        release_mem_object(
            buffer
        );

    kernel_release_result =
        release_kernel(
            kernel
        );

    program_release_result =
        release_program(
            program
        );

    queue_release_result =
        release_queue(
            queue
        );

    context_release_result =
        release_context(
            context
        );

    if (argument_result != 0 ||
        enqueue_result != 0 ||
        finish_result != 0 ||
        buffer_release_result != 0 ||
        kernel_release_result != 0 ||
        program_release_result != 0 ||
        queue_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->execution_ready = 1;
}

static void stn_compute_linux_qualify_opencl_result(
    void * module,
    stn_compute_provider *provider
)
{
    static const char source_text[] =
        "__kernel void stn_qualify(__global uchar *data) { data[0] = (uchar)0x5a; }";

    const char *source;

    stn_opencl_get_platform_ids_fn get_platform_ids;
    stn_opencl_get_device_ids_fn get_device_ids;
    stn_opencl_create_context_fn create_context;
    stn_opencl_release_context_fn release_context;
    stn_opencl_create_command_queue_fn create_queue;
    stn_opencl_release_command_queue_fn release_queue;
    stn_opencl_create_program_with_source_fn create_program;
    stn_opencl_release_program_fn release_program;
    stn_opencl_build_program_fn build_program;
    stn_opencl_create_kernel_fn create_kernel;
    stn_opencl_release_kernel_fn release_kernel;
    stn_opencl_create_buffer_fn create_buffer;
    stn_opencl_release_mem_object_fn release_mem_object;
    stn_opencl_set_kernel_arg_fn set_kernel_arg;
    stn_opencl_enqueue_ndrange_kernel_fn enqueue_ndrange_kernel;
    stn_opencl_enqueue_read_buffer_fn enqueue_read_buffer;
    stn_opencl_finish_fn finish;
    void *platforms[STN_OPENCL_MAX_PLATFORMS];
    void *device;
    void *context;
    void *queue;
    void *program;
    void *kernel;
    void *buffer;
    size_t global_work_size;
    unsigned char result_byte;
    unsigned int platform_count;
    unsigned int i;
    int result;
    int build_result;
    int argument_result;
    int enqueue_result;
    int finish_result;
    int read_result;
    int buffer_release_result;
    int kernel_release_result;
    int program_release_result;
    int queue_release_result;
    int context_release_result;

    if (module == NULL ||
        provider == NULL ||
        !provider->execution_ready ||
        provider->platform_count == 0u ||
        provider->platform_count >
            STN_OPENCL_MAX_PLATFORMS) {
        return;
    }

    get_platform_ids =
        (stn_opencl_get_platform_ids_fn)
        dlsym(module, "clGetPlatformIDs");

    get_device_ids =
        (stn_opencl_get_device_ids_fn)
        dlsym(module, "clGetDeviceIDs");

    create_context =
        (stn_opencl_create_context_fn)
        dlsym(module, "clCreateContext");

    release_context =
        (stn_opencl_release_context_fn)
        dlsym(module, "clReleaseContext");

    create_queue =
        (stn_opencl_create_command_queue_fn)
        dlsym(module, "clCreateCommandQueue");

    release_queue =
        (stn_opencl_release_command_queue_fn)
        dlsym(module, "clReleaseCommandQueue");

    create_program =
        (stn_opencl_create_program_with_source_fn)
        dlsym(module, "clCreateProgramWithSource");

    release_program =
        (stn_opencl_release_program_fn)
        dlsym(module, "clReleaseProgram");

    build_program =
        (stn_opencl_build_program_fn)
        dlsym(module, "clBuildProgram");

    create_kernel =
        (stn_opencl_create_kernel_fn)
        dlsym(module, "clCreateKernel");

    release_kernel =
        (stn_opencl_release_kernel_fn)
        dlsym(module, "clReleaseKernel");

    create_buffer =
        (stn_opencl_create_buffer_fn)
        dlsym(module, "clCreateBuffer");

    release_mem_object =
        (stn_opencl_release_mem_object_fn)
        dlsym(module, "clReleaseMemObject");

    set_kernel_arg =
        (stn_opencl_set_kernel_arg_fn)
        dlsym(module, "clSetKernelArg");

    enqueue_ndrange_kernel =
        (stn_opencl_enqueue_ndrange_kernel_fn)
        dlsym(module, "clEnqueueNDRangeKernel");

    enqueue_read_buffer =
        (stn_opencl_enqueue_read_buffer_fn)
        dlsym(module, "clEnqueueReadBuffer");

    finish =
        (stn_opencl_finish_fn)
        dlsym(module, "clFinish");

    if (get_platform_ids == NULL ||
        get_device_ids == NULL ||
        create_context == NULL ||
        release_context == NULL ||
        create_queue == NULL ||
        release_queue == NULL ||
        create_program == NULL ||
        release_program == NULL ||
        build_program == NULL ||
        create_kernel == NULL ||
        release_kernel == NULL ||
        create_buffer == NULL ||
        release_mem_object == NULL ||
        set_kernel_arg == NULL ||
        enqueue_ndrange_kernel == NULL ||
        enqueue_read_buffer == NULL ||
        finish == NULL) {
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

    device = NULL;

    for (i = 0u;
         i < platform_count;
         ++i) {

        result =
            get_device_ids(
                platforms[i],
                STN_OPENCL_DEVICE_TYPE_GPU,
                1u,
                &device,
                NULL
            );

        if (result ==
            STN_OPENCL_DEVICE_NOT_FOUND) {
            continue;
        }

        if (result != 0 ||
            device == NULL) {
            return;
        }

        break;
    }

    if (device == NULL) {
        return;
    }

    result = 0;

    context =
        create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (context == NULL ||
        result != 0) {
        return;
    }

    result = 0;

    queue =
        create_queue(
            context,
            device,
            0u,
            &result
        );

    if (queue == NULL ||
        result != 0) {

        (void) release_context(
            context
        );

        return;
    }

    source = source_text;
    result = 0;

    program =
        create_program(
            context,
            1u,
            &source,
            NULL,
            &result
        );

    if (program == NULL ||
        result != 0) {

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    build_result =
        build_program(
            program,
            1u,
            &device,
            NULL,
            NULL,
            NULL
        );

    if (build_result != 0) {

        (void) release_program(
            program
        );

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    kernel =
        create_kernel(
            program,
            "stn_qualify",
            &result
        );

    if (kernel == NULL ||
        result != 0) {

        (void) release_program(
            program
        );

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    result = 0;

    buffer =
        create_buffer(
            context,
            STN_OPENCL_MEM_READ_WRITE,
            1u,
            NULL,
            &result
        );

    if (buffer == NULL ||
        result != 0) {

        (void) release_kernel(
            kernel
        );

        (void) release_program(
            program
        );

        (void) release_queue(
            queue
        );

        (void) release_context(
            context
        );

        return;
    }

    argument_result =
        set_kernel_arg(
            kernel,
            0u,
            sizeof(buffer),
            &buffer
        );

    global_work_size = 1u;
    enqueue_result = -1;
    finish_result = -1;
    read_result = -1;
    result_byte = 0u;

    if (argument_result == 0) {
        enqueue_result =
            enqueue_ndrange_kernel(
                queue,
                kernel,
                1u,
                NULL,
                &global_work_size,
                NULL,
                0u,
                NULL,
                NULL
            );
    }

    if (enqueue_result == 0) {
        finish_result =
            finish(
                queue
            );
    }

    if (finish_result == 0) {
        read_result =
            enqueue_read_buffer(
                queue,
                buffer,
                STN_OPENCL_TRUE,
                0u,
                1u,
                &result_byte,
                0u,
                NULL,
                NULL
            );
    }

    buffer_release_result =
        release_mem_object(
            buffer
        );

    kernel_release_result =
        release_kernel(
            kernel
        );

    program_release_result =
        release_program(
            program
        );

    queue_release_result =
        release_queue(
            queue
        );

    context_release_result =
        release_context(
            context
        );

    if (argument_result != 0 ||
        enqueue_result != 0 ||
        finish_result != 0 ||
        read_result != 0 ||
        result_byte != 0x5au ||
        buffer_release_result != 0 ||
        kernel_release_result != 0 ||
        program_release_result != 0 ||
        queue_release_result != 0 ||
        context_release_result != 0) {
        return;
    }

    provider->result_ready = 1;
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
    provider->device_query_ready = 0;
    provider->device_count = 0u;
    provider->device_identity_ready = 0;
    provider->device_name[0] = '\0';
    provider->device_vendor[0] = '\0';
    provider->context_ready = 0;
    provider->queue_ready = 0;
    provider->buffer_ready = 0;
    provider->transfer_ready = 0;
    provider->program_ready = 0;
    provider->build_ready = 0;
    provider->kernel_ready = 0;
    provider->argument_ready = 0;
    provider->execution_ready = 0;
    provider->result_ready = 0;

    if (type ==
        STN_COMPUTE_PROVIDER_OPENCL) {
        stn_compute_linux_qualify_opencl_platforms(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_devices(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_identity(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_context(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_queue(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_buffer(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_transfer(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_program(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_build(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_kernel(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_argument(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_execution(
            module,
            provider
        );

        stn_compute_linux_qualify_opencl_result(
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
