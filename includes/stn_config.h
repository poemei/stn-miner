/* stn-miner\includes\stn_config.h */

#ifndef STN_CONFIG_H
#define STN_CONFIG_H

#include "stn_miner.h"

#define STN_CONFIG_FILENAME "config.json"
#define STN_CONFIG_MAX_FILE_SIZE 4096u

typedef enum stn_config_status {
    STN_CONFIG_OK = 0,
    STN_CONFIG_INVALID_ARGUMENT = 1,
    STN_CONFIG_OPEN_FAILED = 2,
    STN_CONFIG_READ_FAILED = 3,
    STN_CONFIG_INVALID_JSON = 4,
    STN_CONFIG_INVALID_ADDRESS = 5,
    STN_CONFIG_INVALID_HOST = 6,
    STN_CONFIG_INVALID_PORT = 7
} stn_config_status;

stn_config_status stn_config_load(
    const char *path,
    stn_miner_config *config
);

#endif