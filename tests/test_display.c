#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stn_display.h"

static int failures = 0;

void stn_platform_console_clear(void)
{
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

static void fill_work_id(
    uint8_t work_id[STNM_WORK_ID_SIZE],
    uint8_t seed
)
{
    size_t i;

    for (i = 0u;
         i < STNM_WORK_ID_SIZE;
         ++i) {
        work_id[i] =
            (uint8_t) (seed + (uint8_t) i);
    }
}

int main(void)
{
    stn_miner_config config;
    stn_display_state display;
    uint8_t first_work[STNM_WORK_ID_SIZE];
    uint8_t second_work[STNM_WORK_ID_SIZE];

    memset(
        &config,
        0,
        sizeof(config)
    );

    (void) snprintf(
        config.address,
        sizeof(config.address),
        "%s",
        "stn0_f565306974b8aa6174d42d989e8262817b06b024fe1bfb3b0233699e7f26c1b2"
    );

    (void) snprintf(
        config.stratum_host,
        sizeof(config.stratum_host),
        "%s",
        "stratum.stn-chain.org"
    );

    config.stratum_port = 18475u;

    fill_work_id(
        first_work,
        0x10u
    );

    fill_work_id(
        second_work,
        0x80u
    );

    stn_display_init(
        &display,
        &config,
        "GPU"
    );

    stn_display_set_job(
        &display,
        first_work
    );

    stn_display_set_nonce(
        &display,
        4095u
    );

    stn_display_set_hashes(
        &display,
        4096u
    );

    stn_display_set_hashes(
        &display,
        8192u
    );

    check(
        display.hashes_completed == 8192u,
        "first job hashes counted"
    );

    check(
        display.nonce == 4095u,
        "first job nonce visible"
    );

    check(
        display.job_count == 1u,
        "first job visible"
    );

    stn_display_set_result(
        &display,
        "Replaced"
    );

    check(
        display.job_count == 0u,
        "replaced job removed from history"
    );

    stn_display_set_job(
        &display,
        second_work
    );

    stn_display_set_nonce(
        &display,
        0u
    );

    stn_display_set_hashes(
        &display,
        0u
    );

    check(
        display.nonce == 4095u,
        "replacement nonce zero does not blank useful progress"
    );

    check(
        display.hashes_completed == 8192u,
        "replacement hash zero does not reset session total"
    );

    check(
        display.job_count == 1u &&
        strcmp(
            display.jobs[0].result,
            "Mining"
        ) == 0,
        "replacement becomes current mining job"
    );

    stn_display_set_nonce(
        &display,
        8191u
    );

    stn_display_set_hashes(
        &display,
        4096u
    );

    check(
        display.nonce == 8191u,
        "new job nonce advances"
    );

    check(
        display.hashes_completed == 12288u,
        "hash total continues across job turnover"
    );

    stn_display_add_share(
        &display
    );

    check(
        display.total_shares == 1u,
        "share total advances"
    );

    stn_display_set_result(
        &display,
        "Stale"
    );

    check(
        strcmp(
            display.jobs[0].result,
            "Stale"
        ) == 0,
        "stale remains a real terminal result"
    );

    stn_display_set_result(
        &display,
        NULL
    );

    check(
        strcmp(
            display.jobs[0].result,
            "Stale"
        ) == 0,
        "null result rejected without state change"
    );

    if (failures != 0) {
        printf(
            "Display: %d failure(s).\n",
            failures
        );
        return 1;
    }

    printf("Display: all checks passed.\n");
    return 0;
}
