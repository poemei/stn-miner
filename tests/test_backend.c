#include <stdio.h>
#include <string.h>

#include "stn_backend.h"

static int failures = 0;

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
    stn_backend backend;

    memset(
        &backend,
        0,
        sizeof(backend)
    );

    check(
        stn_backend_select(NULL) ==
            STN_BACKEND_INVALID_ARGUMENT,
        "null backend rejected"
    );

    check(
        stn_backend_select(&backend) ==
            STN_BACKEND_OK,
        "backend select"
    );

    check(
        backend.type ==
            STN_BACKEND_TYPE_CPU,
        "CPU selected"
    );

    check(
        backend.name != NULL &&
        strcmp(
            backend.name,
            "CPU"
        ) == 0,
        "CPU name"
    );

    check(
        strcmp(
            stn_backend_type_name(
                STN_BACKEND_TYPE_GPU
            ),
            "GPU"
        ) == 0,
        "GPU type name"
    );

    if (failures != 0) {
        printf(
            "Backend: %d failure(s).\n",
            failures
        );
        return 1;
    }

    printf("Backend: all checks passed.\n");
    return 0;
}
