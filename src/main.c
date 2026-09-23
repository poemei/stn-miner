/* stn-miner\src\main.c */

#include <stdio.h>

#include "stn_config.h"
#include "stn_miner.h"
#include "stn_platform.h"

int main(void)
{
    stn_miner_config config;
    stn_config_status config_status;
    stn_miner_status miner_status;
    stn_platform_status platform_status;
    char config_path[STN_PLATFORM_PATH_MAX];

    platform_status =
        stn_platform_executable_path(
            STN_CONFIG_FILENAME,
            config_path,
            sizeof(config_path)
        );

    if (platform_status !=
        STN_PLATFORM_OK) {

        fprintf(
            stderr,
            "failed to resolve %s beside miner executable\n",
            STN_CONFIG_FILENAME
        );

        return 1;
    }

    config_status = stn_config_load(
        config_path,
        &config
    );

    if (config_status != STN_CONFIG_OK) {
        fprintf(
            stderr,
            "failed to load %s: %d\n",
            config_path,
            (int) config_status
        );

        return 1;
    }

    miner_status = stn_miner_run(
        &config
    );

    if (miner_status != STN_MINER_OK) {
        fprintf(
            stderr,
            "STN Miner exited with status %d\n",
            (int) miner_status
        );

        return 1;
    }

    return 0;
}