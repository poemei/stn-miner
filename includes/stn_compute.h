#ifndef STN_COMPUTE_H
#define STN_COMPUTE_H

#include <stddef.h>

#define STN_COMPUTE_MAX_PROVIDERS 8u
#define STN_COMPUTE_DEVICE_TEXT_MAX 128u

typedef enum stn_compute_status {
    STN_COMPUTE_OK = 0,
    STN_COMPUTE_INVALID_ARGUMENT,
    STN_COMPUTE_PLATFORM_ERROR
} stn_compute_status;

typedef enum stn_compute_provider_type {
    STN_COMPUTE_PROVIDER_OPENCL = 0,
    STN_COMPUTE_PROVIDER_CUDA,
    STN_COMPUTE_PROVIDER_HIP,
    STN_COMPUTE_PROVIDER_LEVEL_ZERO,
    STN_COMPUTE_PROVIDER_VULKAN,
    STN_COMPUTE_PROVIDER_D3D12
} stn_compute_provider_type;

typedef struct stn_compute_provider {
    stn_compute_provider_type type;
    const char *runtime;
    int api_ready;
    int platform_ready;
    size_t platform_count;
    int device_query_ready;
    size_t device_count;
    int device_identity_ready;
    char device_name[STN_COMPUTE_DEVICE_TEXT_MAX];
    char device_vendor[STN_COMPUTE_DEVICE_TEXT_MAX];
    int context_ready;
    int queue_ready;
    int buffer_ready;
    int transfer_ready;
    int program_ready;
    int build_ready;
    int kernel_ready;
    int argument_ready;
    int execution_ready;
    int result_ready;
    int vector_ready;
} stn_compute_provider;

typedef struct stn_compute_inventory {
    stn_compute_provider providers[STN_COMPUTE_MAX_PROVIDERS];
    size_t count;
} stn_compute_inventory;

stn_compute_status stn_compute_detect(
    stn_compute_inventory *inventory
);

const char *stn_compute_provider_name(
    stn_compute_provider_type type
);

#endif
