#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stn_gpu.h"
#include "stn_gpu_platform.h"

#define STN_GPU_SYSFS_DRM_PATH "/sys/class/drm"

static int stn_gpu_linux_is_card(
    const char *name
)
{
    size_t index;

    if (name == NULL) {
        return 0;
    }

    if (strncmp(
            name,
            "card",
            4u
        ) != 0) {
        return 0;
    }

    if (name[4] == '\0') {
        return 0;
    }

    index = 4u;

    while (name[index] != '\0') {
        if (!isdigit(
                (unsigned char) name[index]
            )) {
            return 0;
        }

        index++;
    }

    return 1;
}

static int stn_gpu_linux_read_hex_file(
    const char *path,
    uint32_t *value
)
{
    FILE *file;
    char buffer[32];
    char *end;
    unsigned long parsed;

    if (path == NULL ||
        value == NULL) {
        return 0;
    }

    file = fopen(
        path,
        "r"
    );

    if (file == NULL) {
        return 0;
    }

    if (fgets(
            buffer,
            sizeof(buffer),
            file
        ) == NULL) {

        fclose(
            file
        );

        return 0;
    }

    fclose(
        file
    );

    errno = 0;
    end = NULL;

    parsed = strtoul(
        buffer,
        &end,
        0
    );

    if (errno != 0 ||
        end == buffer ||
        parsed > UINT32_MAX) {
        return 0;
    }

    *value =
        (uint32_t) parsed;

    return 1;
}

static stn_gpu_vendor stn_gpu_linux_vendor(
    uint32_t vendor_id
)
{
    switch (vendor_id) {
        case 0x10DEu:
            return STN_GPU_VENDOR_NVIDIA;

        case 0x1002u:
        case 0x1022u:
            return STN_GPU_VENDOR_AMD;

        case 0x8086u:
            return STN_GPU_VENDOR_INTEL;

        default:
            return STN_GPU_VENDOR_UNKNOWN;
    }
}

static const char *stn_gpu_linux_vendor_name(
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

static void stn_gpu_linux_set_name(
    stn_gpu_device *gpu,
    uint32_t device_id
)
{
    const char *vendor_name;

    if (gpu == NULL) {
        return;
    }

    vendor_name =
        stn_gpu_linux_vendor_name(
            gpu->vendor
        );

    (void) snprintf(
        gpu->name,
        sizeof(gpu->name),
        "%s GPU (PCI device 0x%04X)",
        vendor_name,
        (unsigned int) device_id
    );

    gpu->name[
        sizeof(gpu->name) - 1u
    ] = '\0';
}

stn_gpu_status stn_gpu_platform_detect(
    stn_gpu_inventory *inventory
)
{
    DIR *directory;
    struct dirent *entry;

    if (inventory == NULL) {
        return STN_GPU_INVALID_ARGUMENT;
    }

    inventory->count = 0u;

    directory = opendir(
        STN_GPU_SYSFS_DRM_PATH
    );

    if (directory == NULL) {
        return STN_GPU_PLATFORM_ERROR;
    }

    while ((entry = readdir(directory)) != NULL) {
        char vendor_path[512];
        char device_path[512];

        uint32_t vendor_id;
        uint32_t device_id;

        stn_gpu_device *gpu;

        if (inventory->count >=
            STN_GPU_MAX_DEVICES) {
            break;
        }

        if (!stn_gpu_linux_is_card(
                entry->d_name
            )) {
            continue;
        }

        if (snprintf(
                vendor_path,
                sizeof(vendor_path),
                "%s/%s/device/vendor",
                STN_GPU_SYSFS_DRM_PATH,
                entry->d_name
            ) < 0) {
            continue;
        }

        if (snprintf(
                device_path,
                sizeof(device_path),
                "%s/%s/device/device",
                STN_GPU_SYSFS_DRM_PATH,
                entry->d_name
            ) < 0) {
            continue;
        }

        vendor_id = 0u;
        device_id = 0u;

        if (!stn_gpu_linux_read_hex_file(
                vendor_path,
                &vendor_id
            )) {
            continue;
        }

        if (!stn_gpu_linux_read_hex_file(
                device_path,
                &device_id
            )) {
            continue;
        }

        gpu =
            &inventory->devices[
                inventory->count
            ];

        memset(
            gpu,
            0,
            sizeof(*gpu)
        );

        gpu->vendor =
            stn_gpu_linux_vendor(
                vendor_id
            );

        gpu->device_index =
            (uint32_t) inventory->count;

        stn_gpu_linux_set_name(
            gpu,
            device_id
        );

        inventory->count++;
    }

    closedir(
        directory
    );

    if (inventory->count == 0u) {
        return STN_GPU_NOT_FOUND;
    }

    return STN_GPU_OK;
}