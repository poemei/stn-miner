/* C:\poes_projects\stn-miner\src\stn_cpu.c */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stn_cpu.h"
#include "stn_hash.h"
#include "stn_protocol.h"

static const uint8_t stn_block_id_domain[] =
    "STN-CHAIN:BLOCK:ID:1";

static void stn_cpu_write_nonce(
    uint8_t *block,
    uint64_t nonce
)
{
    stn_protocol_write_u64_be(
        &block[STNM_BLOCK_NONCE_OFFSET],
        nonce
    );
}

static void stn_cpu_hash_header(
    const uint8_t *block,
    uint8_t digest[STN_SHA256_DIGEST_SIZE]
)
{
    stn_sha256_context context;

    stn_sha256_init(&context);

    /*
     * sizeof(stn_block_id_domain) intentionally includes the
     * string literal's terminating 0x00 byte.
     *
     * Qualified PoW input:
     *
     * "STN-CHAIN:BLOCK:ID:1"
     * || 0x00
     * || header[0..167]
     */
    stn_sha256_update(
        &context,
        stn_block_id_domain,
        sizeof(stn_block_id_domain)
    );

    stn_sha256_update(
        &context,
        block,
        STNM_BLOCK_HEADER_LENGTH
    );

    stn_sha256_final(
        &context,
        digest
    );
}

stn_cpu_status stn_cpu_search(
    stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
)
{
    uint8_t original_nonce[8];
    uint8_t digest[STN_SHA256_DIGEST_SIZE];
    uint64_t nonce;

    if (job == NULL ||
        solution == NULL ||
        job->block == NULL ||
        job->block_length < STNM_BLOCK_HEADER_LENGTH) {
        return STN_CPU_INVALID_ARGUMENT;
    }

    if (nonce_start > nonce_end) {
        return STN_CPU_INVALID_ARGUMENT;
    }

    memcpy(
        original_nonce,
        &job->block[STNM_BLOCK_NONCE_OFFSET],
        sizeof(original_nonce)
    );

    nonce = nonce_start;

    for (;;) {
        stn_cpu_write_nonce(
            job->block,
            nonce
        );

        stn_cpu_hash_header(
            job->block,
            digest
        );

        if (stn_hash_meets_target(
                digest,
                job->target)) {

            memcpy(
                solution->work_id,
                job->work_id,
                STNM_WORK_ID_SIZE
            );

            solution->nonce = nonce;

            /*
             * Keep the received job intact. Submission carries the
             * work ID and nonce rather than the mutated block.
             */
            memcpy(
                &job->block[STNM_BLOCK_NONCE_OFFSET],
                original_nonce,
                sizeof(original_nonce)
            );

            return STN_CPU_OK;
        }

        if (nonce == nonce_end) {
            break;
        }

        ++nonce;
    }

    memcpy(
        &job->block[STNM_BLOCK_NONCE_OFFSET],
        original_nonce,
        sizeof(original_nonce)
    );

    return STN_CPU_NO_SOLUTION;
}