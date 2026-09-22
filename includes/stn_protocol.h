/* C:\poes_projects\stn-miner\includes\stn_protocol.h */

#ifndef STN_PROTOCOL_H
#define STN_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#include "stn_miner.h"

#define STNM_MAGIC_0 ((uint8_t) 'S')
#define STNM_MAGIC_1 ((uint8_t) 'T')
#define STNM_MAGIC_2 ((uint8_t) 'N')
#define STNM_MAGIC_3 ((uint8_t) 'M')

#define STNM_JOB_BLOCK_LENGTH_OFFSET 72u
#define STNM_JOB_INITIAL_NONCE_OFFSET 76u
#define STNM_JOB_BLOCK_OFFSET 84u

#define STNM_BLOCK_TARGET_OFFSET 120u
#define STNM_BLOCK_NONCE_OFFSET 152u
#define STNM_BLOCK_HEADER_LENGTH 168u

typedef enum stn_protocol_status {
    STN_PROTOCOL_OK = 0,
    STN_PROTOCOL_INVALID_ARGUMENT = 1,
    STN_PROTOCOL_INVALID_MAGIC = 2,
    STN_PROTOCOL_INVALID_VERSION = 3,
    STN_PROTOCOL_INVALID_TYPE = 4,
    STN_PROTOCOL_INVALID_RESERVED = 5,
    STN_PROTOCOL_INVALID_LENGTH = 6,
    STN_PROTOCOL_INVALID_RESULT = 7
} stn_protocol_status;

uint32_t stn_protocol_read_u32_be(
    const uint8_t bytes[4]
);

uint64_t stn_protocol_read_u64_be(
    const uint8_t bytes[8]
);

void stn_protocol_write_u32_be(
    uint8_t bytes[4],
    uint32_t value
);

void stn_protocol_write_u64_be(
    uint8_t bytes[8],
    uint64_t value
);

stn_protocol_status stn_protocol_parse_job_header(
    const uint8_t header[STNM_JOB_HEADER_SIZE],
    stn_miner_job *job
);

stn_protocol_status stn_protocol_build_submit(
    const stn_miner_solution *solution,
    uint8_t submit[STNM_SUBMIT_SIZE]
);

stn_protocol_status stn_protocol_build_hash_progress(
    const stn_miner_hash_progress *progress,
    uint8_t frame[STNM_HASH_PROGRESS_SIZE]
);

stn_protocol_status stn_protocol_build_address(
    const char address[STN_MINER_ADDRESS_SIZE],
    uint8_t frame[STNM_ADDRESS_SIZE]
);

stn_protocol_status stn_protocol_parse_result(
    const uint8_t response[STNM_RESULT_SIZE],
    stn_miner_result_code *result
);

#endif