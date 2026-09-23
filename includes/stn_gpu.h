#ifndef STN_GPU_H
#define STN_GPU_H

#include <stddef.h>
#include <stdint.h>

#define STN_GPU_NAME_MAX 128u
#define STN_GPU_MAX_DEVICES 16u

typedef enum stn_gpu_status {
    STN_GPU_OK = 0,
    STN_GPU_INVALID_ARGUMENT,
    STN_GPU_NOT_FOUND,
    STN_GPU_PLATFORM_ERROR
} stn_gpu_status;

typedef enum stn_gpu_vendor {
    STN_GPU_VENDOR_UNKNOWN = 0,
    STN_GPU_VENDOR_NVIDIA,
    STN_GPU_VENDOR_AMD,
    STN_GPU_VENDOR_INTEL
} stn_gpu_vendor;

typedef struct stn_gpu_device {
    stn_gpu_vendor vendor;
    char name[STN_GPU_NAME_MAX];
    uint32_t device_index;
} stn_gpu_device;

typedef struct stn_gpu_inventory {
    stn_gpu_device devices[STN_GPU_MAX_DEVICES];
    size_t count;
} stn_gpu_inventory;

stn_gpu_status stn_gpu_detect(
    stn_gpu_inventory *inventory
);

const char *stn_gpu_vendor_name(
    stn_gpu_vendor vendor
);

#endif