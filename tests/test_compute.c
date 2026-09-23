#include <stdio.h>
#include <string.h>

#include "stn_compute.h"

static int failures = 0;

stn_compute_status stn_compute_platform_detect(
    stn_compute_inventory *inventory
)
{
    if (inventory == NULL) {
        return STN_COMPUTE_INVALID_ARGUMENT;
    }

    inventory->count = 2u;

    inventory->providers[0].type =
        STN_COMPUTE_PROVIDER_OPENCL;

    inventory->providers[0].runtime =
        "test-opencl";

    inventory->providers[1].type =
        STN_COMPUTE_PROVIDER_VULKAN;

    inventory->providers[1].runtime =
        "test-vulkan";

    return STN_COMPUTE_OK;
}

static void check(
    int condition,
    const char *name
)
{
    if (!condition) {
        ++failures;
        printf(
            "FAIL: %s\n",
            name
        );
    }
}

int main(void)
{
    stn_compute_inventory inventory;
    stn_compute_status status;

    status =
        stn_compute_detect(
            NULL
        );

    check(
        status ==
            STN_COMPUTE_INVALID_ARGUMENT,
        "null inventory rejected"
    );

    memset(
        &inventory,
        0,
        sizeof(inventory)
    );

    status =
        stn_compute_detect(
            &inventory
        );

    check(
        status == STN_COMPUTE_OK,
        "detect status"
    );

    check(
        inventory.count == 2u,
        "provider count"
    );

    check(
        strcmp(
            stn_compute_provider_name(
                STN_COMPUTE_PROVIDER_OPENCL
            ),
            "OpenCL"
        ) == 0,
        "OpenCL name"
    );

    check(
        strcmp(
            stn_compute_provider_name(
                STN_COMPUTE_PROVIDER_LEVEL_ZERO
            ),
            "Level Zero"
        ) == 0,
        "Level Zero name"
    );

    if (failures != 0) {
        printf(
            "Compute discovery: %d failure(s).\n",
            failures
        );
        return 1;
    }

    printf(
        "Compute discovery: all checks passed.\n"
    );

    return 0;
}
