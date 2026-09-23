#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_gpu.h"
#include "stn_gpu_platform.h"

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

static void stn_gpu_win32_copy_name(
    char destination[STN_GPU_NAME_MAX],
    const char *source
)
{
    size_t length;

    if (destination == NULL) {
        return;
    }

    destination[0] = '\0';

    if (source == NULL) {
        return;
    }

    length = strlen(
        source
    );

    if (length >= STN_GPU_NAME_MAX) {
        length =
            STN_GPU_NAME_MAX - 1u;
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

stn_gpu_status stn_gpu_platform_detect(
    stn_gpu_inventory *inventory
)
{
    DWORD adapter_index;

    if (inventory == NULL) {
        return STN_GPU_INVALID_ARGUMENT;
    }

    inventory->count = 0u;

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
         *
         * Physical GPUs and normal display adapters remain eligible
         * even when they are not currently attached to the desktop.
         */
        if ((device.StateFlags &
             DISPLAY_DEVICE_MIRRORING_DRIVER) != 0u) {
            continue;
        }

        /*
         * A real enumerated adapter should have a device identity.
         * Entries without one are not useful for normalized GPU
         * inventory.
         */
        if (device.DeviceID[0] == '\0') {
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

        stn_gpu_win32_copy_name(
            gpu->name,
            device.DeviceString
        );

        gpu->device_index =
            (uint32_t) inventory->count;

        inventory->count++;
    }

    if (inventory->count == 0u) {
        return STN_GPU_NOT_FOUND;
    }

    return STN_GPU_OK;
}