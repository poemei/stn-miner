/* stn-miner\src\main.c */

#include <stdio.h>

#include "stn_config.h"
#include "stn_miner.h"

int main(void)
{
    stn_miner_config config;
    stn_config_status config_status;
    stn_miner_status miner_status;

    config_status = stn_config_load(
        STN_CONFIG_FILENAME,
        &config
    );

    if (config_status != STN_CONFIG_OK) {
        fprintf(
            stderr,
            "failed to load %s: %d\n",
            STN_CONFIG_FILENAME,
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