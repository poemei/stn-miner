#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_gpu.h"
#include "stn_gpu_platform.h"

#define STN_GPU_WIN32_DEVICE_ID_MAX 256u

static stn_gpu_vendor stn_gpu_win32_detect_vendor(
    const char *device_id
)
{
    if (device_id == NULL) {
        return STN_GPU_VENDOR_UNKNOWN;
    }

    if (strstr(
            device_id,
            "VEN_10DE"
        ) != NULL) {
        return STN_GPU_VENDOR_NVIDIA;
    }

    if (strstr(
            device_id,
            "VEN_1002"
        ) != NULL ||
        strstr(
            device_id,
            "VEN_1022"
        ) != NULL) {
        return STN_GPU_VENDOR_AMD;
    }

    if (strstr(
            device_id,
            "VEN_8086"
        ) != NULL) {
        return STN_GPU_VENDOR_INTEL;
    }

    return STN_GPU_VENDOR_UNKNOWN;
}

static void stn_gpu_win32_copy_text(
    char *destination,
    size_t destination_size,
    const char *source
)
{
    size_t length;

    if (destination == NULL ||
        destination_size == 0u) {
        return;
    }

    destination[0] = '\0';

    if (source == NULL) {
        return;
    }

    length = strlen(
        source
    );

    if (length >= destination_size) {
        length =
            destination_size - 1u;
    }

    if (length > 0u) {
        memcpy(
            destination,
            source,
            length
        );
    }

    destination[length] =
        '\0';
}

static int stn_gpu_win32_is_duplicate(
    char device_ids[
        STN_GPU_MAX_DEVICES
    ][STN_GPU_WIN32_DEVICE_ID_MAX],
    size_t count,
    const char *device_id
)
{
    size_t i;

    if (device_id == NULL ||
        device_id[0] == '\0') {
        return 0;
    }

    for (i = 0u;
         i < count;
         ++i) {

        if (strcmp(
                device_ids[i],
                device_id
            ) == 0) {
            return 1;
        }
    }

    return 0;
}

stn_gpu_status stn_gpu_platform_detect(
    stn_gpu_inventory *inventory
)
{
    DWORD adapter_index;

    char device_ids[
        STN_GPU_MAX_DEVICES
    ][STN_GPU_WIN32_DEVICE_ID_MAX];

    if (inventory == NULL) {
        return STN_GPU_INVALID_ARGUMENT;
    }

    inventory->count = 0u;

    memset(
        device_ids,
        0,
        sizeof(device_ids)
    );

    adapter_index = 0u;

    while (inventory->count <
           STN_GPU_MAX_DEVICES) {

        DISPLAY_DEVICEA device;
        stn_gpu_device *gpu;

        memset(
            &device,
            0,
            sizeof(device)
        );

        device.cb =
            sizeof(device);

        if (!EnumDisplayDevicesA(
                NULL,
                adapter_index,
                &device,
                0u
            )) {
            break;
        }

        adapter_index++;

        /*
         * Ignore software mirroring/display drivers.
         */
        if ((device.StateFlags &
             DISPLAY_DEVICE_MIRRORING_DRIVER) != 0u) {
            continue;
        }

        /*
         * An adapter without a device identity cannot be
         * deterministically de-duplicated or classified.
         */
        if (device.DeviceID[0] == '\0') {
            continue;
        }

        /*
         * Windows may expose more than one display-adapter
         * record for the same physical GPU.
         *
         * DeviceID is used as the normalized identity so
         * duplicate representations are not counted as
         * separate GPUs.
         */
        if (stn_gpu_win32_is_duplicate(
                device_ids,
                inventory->count,
                device.DeviceID
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
            stn_gpu_win32_detect_vendor(
                device.DeviceID
            );

        stn_gpu_win32_copy_text(
            gpu->name,
            sizeof(gpu->name),
            device.DeviceString
        );

        gpu->device_index =
            (uint32_t) inventory->count;

        stn_gpu_win32_copy_text(
            device_ids[
                inventory->count
            ],
            sizeof(
                device_ids[
                    inventory->count
                ]
            ),
            device.DeviceID
        );

        inventory->count++;
    }

    if (inventory->count == 0u) {
        return STN_GPU_NOT_FOUND;
    }

    return STN_GPU_OK;
}