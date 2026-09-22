/* C:\poes_projects\stn-miner\src\stn_protocol.c */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_protocol.h"

uint32_t stn_protocol_read_u32_be(
    const uint8_t bytes[4]
)
{
    return
        ((uint32_t) bytes[0] << 24) |
        ((uint32_t) bytes[1] << 16) |
        ((uint32_t) bytes[2] << 8) |
        ((uint32_t) bytes[3]);
}

uint64_t stn_protocol_read_u64_be(
    const uint8_t bytes[8]
)
{
    return
        ((uint64_t) bytes[0] << 56) |
        ((uint64_t) bytes[1] << 48) |
        ((uint64_t) bytes[2] << 40) |
        ((uint64_t) bytes[3] << 32) |
        ((uint64_t) bytes[4] << 24) |
        ((uint64_t) bytes[5] << 16) |
        ((uint64_t) bytes[6] << 8) |
        ((uint64_t) bytes[7]);
}

void stn_protocol_write_u32_be(
    uint8_t bytes[4],
    uint32_t value
)
{
    bytes[0] = (uint8_t) ((value >> 24) & 0xffu);
    bytes[1] = (uint8_t) ((value >> 16) & 0xffu);
    bytes[2] = (uint8_t) ((value >> 8) & 0xffu);
    bytes[3] = (uint8_t) (value & 0xffu);
}

void stn_protocol_write_u64_be(
    uint8_t bytes[8],
    uint64_t value
)
{
    bytes[0] = (uint8_t) ((value >> 56) & 0xffu);
    bytes[1] = (uint8_t) ((value >> 48) & 0xffu);
    bytes[2] = (uint8_t) ((value >> 40) & 0xffu);
    bytes[3] = (uint8_t) ((value >> 32) & 0xffu);
    bytes[4] = (uint8_t) ((value >> 24) & 0xffu);
    bytes[5] = (uint8_t) ((value >> 16) & 0xffu);
    bytes[6] = (uint8_t) ((value >> 8) & 0xffu);
    bytes[7] = (uint8_t) (value & 0xffu);
}

stn_protocol_status stn_protocol_parse_job_header(
    const uint8_t header[STNM_JOB_HEADER_SIZE],
    stn_miner_job *job
)
{
    uint32_t block_length;

    if (header == NULL || job == NULL) {
        return STN_PROTOCOL_INVALID_ARGUMENT;
    }

    if (header[0] != STNM_MAGIC_0 ||
        header[1] != STNM_MAGIC_1 ||
        header[2] != STNM_MAGIC_2 ||
        header[3] != STNM_MAGIC_3) {
        return STN_PROTOCOL_INVALID_MAGIC;
    }

    if (header[4] != STNM_VERSION) {
        return STN_PROTOCOL_INVALID_VERSION;
    }

    if (header[5] != STNM_TYPE_JOB) {
        return STN_PROTOCOL_INVALID_TYPE;
    }

    if (header[6] != 0u || header[7] != 0u) {
        return STN_PROTOCOL_INVALID_RESERVED;
    }

    block_length = stn_protocol_read_u32_be(
        &header[STNM_JOB_BLOCK_LENGTH_OFFSET]
    );

    if (block_length < STNM_BLOCK_HEADER_LENGTH) {
        return STN_PROTOCOL_INVALID_LENGTH;
    }

    memcpy(
        job->work_id,
        &header[8],
        STNM_WORK_ID_SIZE
    );

    memcpy(
        job->target,
        &header[40],
        STNM_TARGET_SIZE
    );

    job->block_length = block_length;

    job->initial_nonce = stn_protocol_read_u64_be(
        &header[STNM_JOB_INITIAL_NONCE_OFFSET]
    );

    return STN_PROTOCOL_OK;
}

stn_protocol_status stn_protocol_build_submit(
    const stn_miner_solution *solution,
    uint8_t submit[STNM_SUBMIT_SIZE]
)
{
    if (solution == NULL || submit == NULL) {
        return STN_PROTOCOL_INVALID_ARGUMENT;
    }

    memset(
        submit,
        0,
        STNM_SUBMIT_SIZE
    );

    submit[0] = STNM_MAGIC_0;
    submit[1] = STNM_MAGIC_1;
    submit[2] = STNM_MAGIC_2;
    submit[3] = STNM_MAGIC_3;

    submit[4] = STNM_VERSION;
    submit[5] = STNM_TYPE_SUBMIT;

    submit[6] = 0u;
    submit[7] = 0u;

    memcpy(
        &submit[8],
        solution->work_id,
        STNM_WORK_ID_SIZE
    );

    stn_protocol_write_u64_be(
        &submit[40],
        solution->nonce
    );

    return STN_PROTOCOL_OK;
}

stn_protocol_status stn_protocol_build_hash_progress(
    const stn_miner_hash_progress *progress,
    uint8_t frame[STNM_HASH_PROGRESS_SIZE]
)
{
    if (progress == NULL || frame == NULL) {
        return STN_PROTOCOL_INVALID_ARGUMENT;
    }

    memset(
        frame,
        0,
        STNM_HASH_PROGRESS_SIZE
    );

    frame[0] = STNM_MAGIC_0;
    frame[1] = STNM_MAGIC_1;
    frame[2] = STNM_MAGIC_2;
    frame[3] = STNM_MAGIC_3;

    frame[4] = STNM_VERSION;
    frame[5] = STNM_TYPE_HASH_PROGRESS;

    frame[6] = 0u;
    frame[7] = 0u;

    memcpy(
        &frame[8],
        progress->work_id,
        STNM_WORK_ID_SIZE
    );

    stn_protocol_write_u64_be(
        &frame[40],
        progress->hashes_completed
    );

    stn_protocol_write_u64_be(
        &frame[48],
        progress->elapsed_ms
    );

    return STN_PROTOCOL_OK;
}

stn_protocol_status stn_protocol_build_address(
    const char address[STN_MINER_ADDRESS_SIZE],
    uint8_t frame[STNM_ADDRESS_SIZE]
)
{
    size_t i;

    if (address == NULL || frame == NULL) {
        return STN_PROTOCOL_INVALID_ARGUMENT;
    }

    if (strlen(address) != STN_MINER_ADDRESS_LENGTH) {
        return STN_PROTOCOL_INVALID_LENGTH;
    }

    if (memcmp(
            address,
            "stn0_",
            5u
        ) != 0) {
        return STN_PROTOCOL_INVALID_ARGUMENT;
    }

    for (i = 5u; i < STN_MINER_ADDRESS_LENGTH; ++i) {
        char c;

        c = address[i];

        if (!((c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f'))) {
            return STN_PROTOCOL_INVALID_ARGUMENT;
        }
    }

    memset(
        frame,
        0,
        STNM_ADDRESS_SIZE
    );

    frame[0] = STNM_MAGIC_0;
    frame[1] = STNM_MAGIC_1;
    frame[2] = STNM_MAGIC_2;
    frame[3] = STNM_MAGIC_3;

    frame[4] = STNM_VERSION;
    frame[5] = STNM_TYPE_ADDRESS;

    frame[6] = 0u;
    frame[7] = 0u;

    memcpy(
        &frame[8],
        address,
        STN_MINER_ADDRESS_LENGTH
    );

    return STN_PROTOCOL_OK;
}

stn_protocol_status stn_protocol_parse_result(
    const uint8_t response[STNM_RESULT_SIZE],
    stn_miner_result_code *result
)
{
    uint32_t code;

    if (response == NULL || result == NULL) {
        return STN_PROTOCOL_INVALID_ARGUMENT;
    }

    if (response[0] != STNM_MAGIC_0 ||
        response[1] != STNM_MAGIC_1 ||
        response[2] != STNM_MAGIC_2 ||
        response[3] != STNM_MAGIC_3) {
        return STN_PROTOCOL_INVALID_MAGIC;
    }

    if (response[4] != STNM_VERSION) {
        return STN_PROTOCOL_INVALID_VERSION;
    }

    if (response[5] != STNM_TYPE_RESULT) {
        return STN_PROTOCOL_INVALID_TYPE;
    }

    if (response[6] != 0u || response[7] != 0u) {
        return STN_PROTOCOL_INVALID_RESERVED;
    }

    code = stn_protocol_read_u32_be(
        &response[8]
    );

    if (code > (uint32_t) STN_MINER_RESULT_MALFORMED) {
        return STN_PROTOCOL_INVALID_RESULT;
    }

    *result = (stn_miner_result_code) code;

    return STN_PROTOCOL_OK;
}