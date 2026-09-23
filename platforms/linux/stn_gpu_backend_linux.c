#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_gpu_backend.h"
#include "stn_gpu_backend_platform.h"

#define STN_OPENCL_DEVICE_TYPE_GPU (1ull << 2)
#define STN_OPENCL_MEM_READ_WRITE (1ull << 0)
#define STN_OPENCL_TRUE 1u
#define STN_GPU_BACKEND_HEADER_SIZE 168u
#define STN_GPU_BACKEND_TARGET_SIZE 32u
#define STN_GPU_BACKEND_MAX_CHUNK 4096u

typedef int (*stn_cl_get_platform_ids_fn)(
    unsigned int,
    void **,
    unsigned int *
);

typedef int (*stn_cl_get_device_ids_fn)(
    void *,
    unsigned long long,
    unsigned int,
    void **,
    unsigned int *
);

typedef void *(*stn_cl_create_context_fn)(
    const intptr_t *,
    unsigned int,
    void * const *,
    void *,
    void *,
    int *
);

typedef int (*stn_cl_release_context_fn)(
    void *
);

typedef void *(*stn_cl_create_queue_fn)(
    void *,
    void *,
    unsigned long long,
    int *
);

typedef int (*stn_cl_release_queue_fn)(
    void *
);

typedef void *(*stn_cl_create_program_fn)(
    void *,
    unsigned int,
    const char **,
    const size_t *,
    int *
);

typedef int (*stn_cl_build_program_fn)(
    void *,
    unsigned int,
    void * const *,
    const char *,
    void *,
    void *
);

typedef int (*stn_cl_release_program_fn)(
    void *
);

typedef void *(*stn_cl_create_kernel_fn)(
    void *,
    const char *,
    int *
);

typedef int (*stn_cl_release_kernel_fn)(
    void *
);

typedef void *(*stn_cl_create_buffer_fn)(
    void *,
    unsigned long long,
    size_t,
    void *,
    int *
);

typedef int (*stn_cl_release_mem_fn)(
    void *
);

typedef int (*stn_cl_set_kernel_arg_fn)(
    void *,
    unsigned int,
    size_t,
    const void *
);

typedef int (*stn_cl_enqueue_write_fn)(
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

typedef int (*stn_cl_enqueue_read_fn)(
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

typedef int (*stn_cl_enqueue_kernel_fn)(
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

typedef int (*stn_cl_finish_fn)(
    void *
);

typedef struct stn_gpu_backend_state {
    int ready;
    void * module;
    void *context;
    void *queue;
    void *program;
    void *kernel;
    void *header_buffer;
    void *target_buffer;
    void *match_buffer;

    stn_cl_get_platform_ids_fn get_platform_ids;
    stn_cl_get_device_ids_fn get_device_ids;
    stn_cl_create_context_fn create_context;
    stn_cl_release_context_fn release_context;
    stn_cl_create_queue_fn create_queue;
    stn_cl_release_queue_fn release_queue;
    stn_cl_create_program_fn create_program;
    stn_cl_build_program_fn build_program;
    stn_cl_release_program_fn release_program;
    stn_cl_create_kernel_fn create_kernel;
    stn_cl_release_kernel_fn release_kernel;
    stn_cl_create_buffer_fn create_buffer;
    stn_cl_release_mem_fn release_mem;
    stn_cl_set_kernel_arg_fn set_kernel_arg;
    stn_cl_enqueue_write_fn enqueue_write;
    stn_cl_enqueue_read_fn enqueue_read;
    stn_cl_enqueue_kernel_fn enqueue_kernel;
    stn_cl_finish_fn finish;
} stn_gpu_backend_state;

static stn_gpu_backend_state state;

stn_gpu_backend_status stn_gpu_backend_platform_prepare(
    const char *kernel_source
)
{
    void *platform;
    void *device;
    const char *source;
    unsigned int count;
    int result;

    if (kernel_source == NULL ||
        kernel_source[0] == '\0') {
        return STN_GPU_BACKEND_INVALID_ARGUMENT;
    }

    if (state.ready) {
        return STN_GPU_BACKEND_OK;
    }

    memset(
        &state,
        0,
        sizeof(state)
    );

    state.module =
        dlopen("libOpenCL.so.1", RTLD_LAZY | RTLD_LOCAL);

    if (state.module == NULL) {
        return STN_GPU_BACKEND_UNAVAILABLE;
    }

    state.get_platform_ids =
        (stn_cl_get_platform_ids_fn)
        dlsym(state.module, "clGetPlatformIDs");

    state.get_device_ids =
        (stn_cl_get_device_ids_fn)
        dlsym(state.module, "clGetDeviceIDs");

    state.create_context =
        (stn_cl_create_context_fn)
        dlsym(state.module, "clCreateContext");

    state.release_context =
        (stn_cl_release_context_fn)
        dlsym(state.module, "clReleaseContext");

    state.create_queue =
        (stn_cl_create_queue_fn)
        dlsym(state.module, "clCreateCommandQueue");

    state.release_queue =
        (stn_cl_release_queue_fn)
        dlsym(state.module, "clReleaseCommandQueue");

    state.create_program =
        (stn_cl_create_program_fn)
        dlsym(state.module, "clCreateProgramWithSource");

    state.build_program =
        (stn_cl_build_program_fn)
        dlsym(state.module, "clBuildProgram");

    state.release_program =
        (stn_cl_release_program_fn)
        dlsym(state.module, "clReleaseProgram");

    state.create_kernel =
        (stn_cl_create_kernel_fn)
        dlsym(state.module, "clCreateKernel");

    state.release_kernel =
        (stn_cl_release_kernel_fn)
        dlsym(state.module, "clReleaseKernel");

    state.create_buffer =
        (stn_cl_create_buffer_fn)
        dlsym(state.module, "clCreateBuffer");

    state.release_mem =
        (stn_cl_release_mem_fn)
        dlsym(state.module, "clReleaseMemObject");

    state.set_kernel_arg =
        (stn_cl_set_kernel_arg_fn)
        dlsym(state.module, "clSetKernelArg");

    state.enqueue_write =
        (stn_cl_enqueue_write_fn)
        dlsym(state.module, "clEnqueueWriteBuffer");

    state.enqueue_read =
        (stn_cl_enqueue_read_fn)
        dlsym(state.module, "clEnqueueReadBuffer");

    state.enqueue_kernel =
        (stn_cl_enqueue_kernel_fn)
        dlsym(state.module, "clEnqueueNDRangeKernel");

    state.finish =
        (stn_cl_finish_fn)
        dlsym(state.module, "clFinish");

    if (state.get_platform_ids == NULL ||
        state.get_device_ids == NULL ||
        state.create_context == NULL ||
        state.release_context == NULL ||
        state.create_queue == NULL ||
        state.release_queue == NULL ||
        state.create_program == NULL ||
        state.build_program == NULL ||
        state.release_program == NULL ||
        state.create_kernel == NULL ||
        state.release_kernel == NULL ||
        state.create_buffer == NULL ||
        state.release_mem == NULL ||
        state.set_kernel_arg == NULL ||
        state.enqueue_write == NULL ||
        state.enqueue_read == NULL ||
        state.enqueue_kernel == NULL ||
        state.finish == NULL) {
        return STN_GPU_BACKEND_UNAVAILABLE;
    }

    platform = NULL;
    count = 0u;

    result =
        state.get_platform_ids(
            1u,
            &platform,
            &count
        );

    if (result != 0 ||
        count == 0u ||
        platform == NULL) {
        return STN_GPU_BACKEND_UNAVAILABLE;
    }

    device = NULL;
    count = 0u;

    result =
        state.get_device_ids(
            platform,
            STN_OPENCL_DEVICE_TYPE_GPU,
            1u,
            &device,
            &count
        );

    if (result != 0 ||
        count == 0u ||
        device == NULL) {
        return STN_GPU_BACKEND_UNAVAILABLE;
    }

    result = 0;

    state.context =
        state.create_context(
            NULL,
            1u,
            &device,
            NULL,
            NULL,
            &result
        );

    if (state.context == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result = 0;

    state.queue =
        state.create_queue(
            state.context,
            device,
            0u,
            &result
        );

    if (state.queue == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    source = kernel_source;
    result = 0;

    state.program =
        state.create_program(
            state.context,
            1u,
            &source,
            NULL,
            &result
        );

    if (state.program == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.build_program(
            state.program,
            1u,
            &device,
            NULL,
            NULL,
            NULL
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result = 0;

    state.kernel =
        state.create_kernel(
            state.program,
            "stn_mine",
            &result
        );

    if (state.kernel == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result = 0;

    state.header_buffer =
        state.create_buffer(
            state.context,
            STN_OPENCL_MEM_READ_WRITE,
            STN_GPU_BACKEND_HEADER_SIZE,
            NULL,
            &result
        );

    if (state.header_buffer == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result = 0;

    state.target_buffer =
        state.create_buffer(
            state.context,
            STN_OPENCL_MEM_READ_WRITE,
            STN_GPU_BACKEND_TARGET_SIZE,
            NULL,
            &result
        );

    if (state.target_buffer == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result = 0;

    state.match_buffer =
        state.create_buffer(
            state.context,
            STN_OPENCL_MEM_READ_WRITE,
            STN_GPU_BACKEND_MAX_CHUNK,
            NULL,
            &result
        );

    if (state.match_buffer == NULL ||
        result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.set_kernel_arg(
            state.kernel,
            0u,
            sizeof(state.header_buffer),
            &state.header_buffer
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.set_kernel_arg(
            state.kernel,
            1u,
            sizeof(state.target_buffer),
            &state.target_buffer
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.set_kernel_arg(
            state.kernel,
            3u,
            sizeof(state.match_buffer),
            &state.match_buffer
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    state.ready = 1;
    return STN_GPU_BACKEND_OK;
}

stn_gpu_backend_status stn_gpu_backend_platform_search(
    const stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
)
{
    unsigned char matches[STN_GPU_BACKEND_MAX_CHUNK];
    uint64_t count64;
    size_t count;
    size_t i;
    int result;

    if (!state.ready) {
        return STN_GPU_BACKEND_UNAVAILABLE;
    }

    if (job == NULL ||
        solution == NULL ||
        job->block == NULL ||
        job->block_length < STN_GPU_BACKEND_HEADER_SIZE ||
        nonce_start > nonce_end) {
        return STN_GPU_BACKEND_INVALID_ARGUMENT;
    }

    count64 =
        (nonce_end - nonce_start) + 1u;

    if (count64 == 0u ||
        count64 > STN_GPU_BACKEND_MAX_CHUNK) {
        return STN_GPU_BACKEND_INVALID_ARGUMENT;
    }

    count =
        (size_t) count64;

    result =
        state.enqueue_write(
            state.queue,
            state.header_buffer,
            STN_OPENCL_TRUE,
            0u,
            STN_GPU_BACKEND_HEADER_SIZE,
            job->block,
            0u,
            NULL,
            NULL
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.enqueue_write(
            state.queue,
            state.target_buffer,
            STN_OPENCL_TRUE,
            0u,
            STN_GPU_BACKEND_TARGET_SIZE,
            job->target,
            0u,
            NULL,
            NULL
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.set_kernel_arg(
            state.kernel,
            2u,
            sizeof(nonce_start),
            &nonce_start
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.enqueue_kernel(
            state.queue,
            state.kernel,
            1u,
            NULL,
            &count,
            NULL,
            0u,
            NULL,
            NULL
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    result =
        state.finish(
            state.queue
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    memset(
        matches,
        0,
        count
    );

    result =
        state.enqueue_read(
            state.queue,
            state.match_buffer,
            STN_OPENCL_TRUE,
            0u,
            count,
            matches,
            0u,
            NULL,
            NULL
        );

    if (result != 0) {
        return STN_GPU_BACKEND_ERROR;
    }

    for (i = 0u;
         i < count;
         ++i) {

        if (matches[i] != 0u) {
            memcpy(
                solution->work_id,
                job->work_id,
                STNM_WORK_ID_SIZE
            );

            solution->nonce =
                nonce_start +
                (uint64_t) i;

            return STN_GPU_BACKEND_OK;
        }
    }

    return STN_GPU_BACKEND_NO_SOLUTION;
}
