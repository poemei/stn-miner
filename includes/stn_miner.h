#ifndef STN_MINER_H
#define STN_MINER_H

#include <stddef.h>
#include <stdint.h>

#define STN_MINER_ADDRESS_LENGTH 69u
#define STN_MINER_ADDRESS_SIZE 70u

#define STN_MINER_HOST_MAX 256u
#define STN_MINER_DEFAULT_PORT 18475u

#define STNM_MAGIC_SIZE 4u
#define STNM_WORK_ID_SIZE 32u
#define STNM_TARGET_SIZE 32u
#define STNM_JOB_HEADER_SIZE 116u
#define STNM_SUBMIT_SIZE 48u
#define STNM_RESULT_SIZE 12u
#define STNM_HASH_PROGRESS_SIZE 56u
#define STNM_ADDRESS_SIZE 77u

#define STNM_VERSION 1u

#define STNM_TYPE_JOB 1u
#define STNM_TYPE_SUBMIT 2u
#define STNM_TYPE_RESULT 3u
#define STNM_TYPE_HASH_PROGRESS 4u
#define STNM_TYPE_ADDRESS 5u

typedef enum stn_miner_status {
    STN_MINER_OK = 0,
    STN_MINER_ERROR = 1,
    STN_MINER_INVALID_ARGUMENT = 2,
    STN_MINER_CONNECT_FAILED = 3,
    STN_MINER_IO_FAILED = 4,
    STN_MINER_PROTOCOL_ERROR = 5
} stn_miner_status;

typedef enum stn_miner_result_code {
    STN_MINER_RESULT_ACCEPTED = 0,
    STN_MINER_RESULT_REJECTED = 1,
    STN_MINER_RESULT_STALE = 2,
    STN_MINER_RESULT_PROVIDER = 3,
    STN_MINER_RESULT_MALFORMED = 4
} stn_miner_result_code;

typedef struct stn_miner_config {
    char address[STN_MINER_ADDRESS_SIZE];
    char stratum_host[STN_MINER_HOST_MAX];
    uint16_t stratum_port;
} stn_miner_config;

typedef struct stn_miner_job {
    uint8_t work_id[STNM_WORK_ID_SIZE];
    uint8_t target[STNM_TARGET_SIZE];
    uint8_t chain_target[STNM_TARGET_SIZE];

    uint8_t *block;
    uint32_t block_length;

    uint64_t initial_nonce;
} stn_miner_job;

typedef struct stn_miner_solution {
    uint8_t work_id[STNM_WORK_ID_SIZE];
    uint64_t nonce;
} stn_miner_solution;

typedef struct stn_miner_hash_progress {
    uint8_t work_id[STNM_WORK_ID_SIZE];
    uint64_t hashes_completed;
    uint64_t elapsed_ms;
} stn_miner_hash_progress;

stn_miner_status stn_miner_config_default(
    stn_miner_config *config
);

stn_miner_status stn_miner_run(
    const stn_miner_config *config
);

#endif