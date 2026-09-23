#include <stdio.h>
#include <string.h>

#include "stn_gpu.h"

static int failures = 0;

stn_gpu_status stn_gpu_platform_detect(
    stn_gpu_inventory *inventory
)
{
    if (inventory == NULL) {
        return STN_GPU_INVALID_ARGUMENT;
    }

    memset(
        inventory,
        0,
        sizeof(*inventory)
    );

    inventory->count = 1u;
    inventory->devices[0].vendor =
        STN_GPU_VENDOR_INTEL;
    inventory->devices[0].device_index = 0u;

    (void) snprintf(
        inventory->devices[0].name,
        sizeof(inventory->devices[0].name),
        "%s",
        "Test GPU"
    );

    return STN_GPU_OK;
}

static void check(
    int condition,
    const char *name
)
{
    if (!condition) {
        ++failures;
        printf("FAIL: %s\n", name);
    }
}

int main(void)
{
    stn_gpu_inventory inventory;
    stn_gpu_status status;

    status = stn_gpu_detect(NULL);
    check(
        status == STN_GPU_INVALID_ARGUMENT,
        "null inventory rejected"
    );

    memset(
        &inventory,
        0,
        sizeof(inventory)
    );

    status = stn_gpu_detect(&inventory);

    check(status == STN_GPU_OK, "detect status");
    check(inventory.count == 1u, "inventory count");
    check(
        inventory.devices[0].vendor ==
            STN_GPU_VENDOR_INTEL,
        "vendor preserved"
    );
    check(
        strcmp(
            stn_gpu_vendor_name(STN_GPU_VENDOR_NVIDIA),
            "NVIDIA"
        ) == 0,
        "NVIDIA name"
    );
    check(
        strcmp(
            stn_gpu_vendor_name(STN_GPU_VENDOR_AMD),
            "AMD"
        ) == 0,
        "AMD name"
    );
    check(
        strcmp(
            stn_gpu_vendor_name(STN_GPU_VENDOR_INTEL),
            "Intel"
        ) == 0,
        "Intel name"
    );
    check(
        strcmp(
            stn_gpu_vendor_name(STN_GPU_VENDOR_UNKNOWN),
            "Unknown"
        ) == 0,
        "unknown name"
    );

    if (failures != 0) {
        printf(
            "GPU detection: %d failure(s).\n",
            failures
        );
        return 1;
    }

    printf("GPU detection: all checks passed.\n");
    return 0;
}
