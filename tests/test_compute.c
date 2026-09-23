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

    inventory->providers[0].api_ready = 1;
    inventory->providers[0].platform_ready = 1;
    inventory->providers[0].platform_count = 2u;
    inventory->providers[0].device_query_ready = 1;
    inventory->providers[0].device_count = 1u;
    inventory->providers[0].device_identity_ready = 1;
    (void) strcpy(
        inventory->providers[0].device_name,
        "Test GPU"
    );
    (void) strcpy(
        inventory->providers[0].device_vendor,
        "Test Vendor"
    );
    inventory->providers[0].context_ready = 1;
    inventory->providers[0].queue_ready = 1;

    inventory->providers[1].type =
        STN_COMPUTE_PROVIDER_VULKAN;

    inventory->providers[1].runtime =
        "test-vulkan";

    inventory->providers[1].api_ready = 0;
    inventory->providers[1].platform_ready = 0;
    inventory->providers[1].platform_count = 0u;
    inventory->providers[1].device_query_ready = 0;
    inventory->providers[1].device_count = 0u;
    inventory->providers[1].device_identity_ready = 0;
    inventory->providers[1].device_name[0] = '\0';
    inventory->providers[1].device_vendor[0] = '\0';
    inventory->providers[1].context_ready = 0;
    inventory->providers[1].queue_ready = 0;

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
        inventory.providers[0].api_ready == 1,
        "ready provider preserved"
    );

    check(
        inventory.providers[1].api_ready == 0,
        "unready provider preserved"
    );

    check(
        inventory.providers[0].platform_ready == 1,
        "OpenCL platform readiness preserved"
    );

    check(
        inventory.providers[0].platform_count == 2u,
        "OpenCL platform count preserved"
    );

    check(
        inventory.providers[0].device_query_ready == 1,
        "OpenCL device query readiness preserved"
    );

    check(
        inventory.providers[0].device_count == 1u,
        "OpenCL GPU device count preserved"
    );

    check(
        inventory.providers[0].device_identity_ready == 1,
        "OpenCL device identity readiness preserved"
    );

    check(
        strcmp(
            inventory.providers[0].device_name,
            "Test GPU"
        ) == 0,
        "OpenCL device name preserved"
    );

    check(
        strcmp(
            inventory.providers[0].device_vendor,
            "Test Vendor"
        ) == 0,
        "OpenCL device vendor preserved"
    );

    check(
        inventory.providers[0].context_ready == 1,
        "OpenCL context readiness preserved"
    );

    check(
        inventory.providers[0].queue_ready == 1,
        "OpenCL command queue readiness preserved"
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
