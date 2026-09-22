/* C:\poes_projects\stn-miner\src\stn_miner.c */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stn_cpu.h"
#include "stn_display.h"
#include "stn_log.h"
#include "stn_miner.h"
#include "stn_platform.h"
#include "stn_protocol.h"

#define STN_MINER_MAX_BLOCK_LENGTH 1051880u
#define STN_MINER_RECONNECT_DELAY_MS 1000u

static void stn_miner_job_clear(
    stn_miner_job *job
)
{
    if (job == NULL) {
        return;
    }

    if (job->block != NULL) {
        free(
            job->block
        );
    }

    memset(
        job,
        0,
        sizeof(*job)
    );
}

static void stn_miner_log_work_id(
    const char *prefix,
    const uint8_t work_id[STNM_WORK_ID_SIZE]
)
{
    static const char hex[] =
        "0123456789abcdef";

    char text[
        (STNM_WORK_ID_SIZE * 2u) + 1u
    ];

    size_t i;

    if (prefix == NULL ||
        work_id == NULL) {
        return;
    }

    for (i = 0u;
         i < STNM_WORK_ID_SIZE;
         ++i) {

        text[i * 2u] =
            hex[
                (work_id[i] >> 4) & 0x0fu
            ];

        text[
            (i * 2u) + 1u
        ] =
            hex[
                work_id[i] & 0x0fu
            ];
    }

    text[
        STNM_WORK_ID_SIZE * 2u
    ] = '\0';

    stn_log_write(
        "%s work_id=%s",
        prefix,
        text
    );
}

static stn_miner_status stn_miner_send_address(
    stn_socket *socket,
    const char address[STN_MINER_ADDRESS_SIZE]
)
{
    uint8_t frame[STNM_ADDRESS_SIZE];

    stn_protocol_status protocol_status;
    stn_platform_status platform_status;

    if (socket == NULL ||
        address == NULL) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    protocol_status =
        stn_protocol_build_address(
            address,
            frame
        );

    if (protocol_status != STN_PROTOCOL_OK) {
        stn_log_write(
            "TX ADDRESS_BUILD_FAILED protocol_status=%d",
            (int) protocol_status
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    stn_log_write(
        "TX ADDRESS bytes=%u type=%u address=%s",
        (unsigned int) STNM_ADDRESS_SIZE,
        (unsigned int) STNM_TYPE_ADDRESS,
        address
    );

    platform_status =
        stn_platform_socket_send_all(
            socket,
            frame,
            sizeof(frame)
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "TX ADDRESS_FAILED platform_status=%d",
            (int) platform_status
        );

        return STN_MINER_IO_FAILED;
    }

    stn_log_write(
        "TX ADDRESS_OK"
    );

    return STN_MINER_OK;
}

static stn_miner_status stn_miner_receive_job(
    stn_socket *socket,
    stn_miner_job *job
)
{
    uint8_t header[STNM_JOB_HEADER_SIZE];

    stn_platform_status platform_status;
    stn_protocol_status protocol_status;

    uint64_t block_nonce;

    if (socket == NULL ||
        job == NULL) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    memset(
        job,
        0,
        sizeof(*job)
    );

    platform_status =
        stn_platform_socket_receive_all(
            socket,
            header,
            sizeof(header)
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "RX JOB_HEADER_FAILED platform_status=%d",
            (int) platform_status
        );

        return STN_MINER_IO_FAILED;
    }

    stn_log_write(
        "RX HEADER bytes=%u magic=%02x%02x%02x%02x "
        "version=%u type=%u reserved=%u,%u",
        (unsigned int) STNM_JOB_HEADER_SIZE,
        (unsigned int) header[0],
        (unsigned int) header[1],
        (unsigned int) header[2],
        (unsigned int) header[3],
        (unsigned int) header[4],
        (unsigned int) header[5],
        (unsigned int) header[6],
        (unsigned int) header[7]
    );

    protocol_status =
        stn_protocol_parse_job_header(
            header,
            job
        );

    if (protocol_status != STN_PROTOCOL_OK) {
        stn_log_write(
            "RX JOB_PARSE_FAILED protocol_status=%d",
            (int) protocol_status
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    if (job->block_length >
        STN_MINER_MAX_BLOCK_LENGTH) {

        stn_log_write(
            "RX JOB_INVALID_LENGTH block_length=%u",
            (unsigned int) job->block_length
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    job->block =
        (uint8_t *) malloc(
            (size_t) job->block_length
        );

    if (job->block == NULL) {
        stn_log_write(
            "RX JOB_ALLOC_FAILED block_length=%u",
            (unsigned int) job->block_length
        );

        return STN_MINER_ERROR;
    }

    platform_status =
        stn_platform_socket_receive_all(
            socket,
            job->block,
            (size_t) job->block_length
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "RX JOB_BLOCK_FAILED platform_status=%d",
            (int) platform_status
        );

        stn_miner_job_clear(
            job
        );

        return STN_MINER_IO_FAILED;
    }

    if (memcmp(
            job->target,
            &job->block[
                STNM_BLOCK_TARGET_OFFSET
            ],
            STNM_TARGET_SIZE
        ) != 0) {

        stn_log_write(
            "RX JOB_TARGET_MISMATCH"
        );

        stn_miner_job_clear(
            job
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    block_nonce =
        stn_protocol_read_u64_be(
            &job->block[
                STNM_BLOCK_NONCE_OFFSET
            ]
        );

    if (block_nonce !=
        job->initial_nonce) {

        stn_log_write(
            "RX JOB_NONCE_MISMATCH header_nonce=%llu block_nonce=%llu",
            (unsigned long long)
                job->initial_nonce,
            (unsigned long long)
                block_nonce
        );

        stn_miner_job_clear(
            job
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    stn_miner_log_work_id(
        "RX JOB_OK",
        job->work_id
    );

    stn_log_write(
        "RX JOB_DETAILS block_length=%u initial_nonce=%llu",
        (unsigned int)
            job->block_length,
        (unsigned long long)
            job->initial_nonce
    );

    return STN_MINER_OK;
}

static stn_miner_status stn_miner_submit_solution(
    stn_socket *socket,
    const stn_miner_solution *solution,
    stn_miner_result_code *result
)
{
    uint8_t request[STNM_SUBMIT_SIZE];
    uint8_t response[STNM_RESULT_SIZE];

    stn_protocol_status protocol_status;
    stn_platform_status platform_status;

    if (socket == NULL ||
        solution == NULL ||
        result == NULL) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    protocol_status =
        stn_protocol_build_submit(
            solution,
            request
        );

    if (protocol_status != STN_PROTOCOL_OK) {
        stn_log_write(
            "TX SUBMIT_BUILD_FAILED protocol_status=%d",
            (int) protocol_status
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    stn_miner_log_work_id(
        "TX SUBMIT",
        solution->work_id
    );

    stn_log_write(
        "TX SUBMIT_NONCE nonce=%llu bytes=%u",
        (unsigned long long)
            solution->nonce,
        (unsigned int)
            STNM_SUBMIT_SIZE
    );

    platform_status =
        stn_platform_socket_send_all(
            socket,
            request,
            sizeof(request)
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "TX SUBMIT_FAILED platform_status=%d",
            (int) platform_status
        );

        return STN_MINER_IO_FAILED;
    }

    stn_log_write(
        "TX SUBMIT_OK"
    );

    platform_status =
        stn_platform_socket_receive_all(
            socket,
            response,
            sizeof(response)
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "RX RESULT_FAILED platform_status=%d",
            (int) platform_status
        );

        return STN_MINER_IO_FAILED;
    }

    stn_log_write(
        "RX RESULT_FRAME magic=%02x%02x%02x%02x "
        "version=%u type=%u reserved=%u,%u",
        (unsigned int) response[0],
        (unsigned int) response[1],
        (unsigned int) response[2],
        (unsigned int) response[3],
        (unsigned int) response[4],
        (unsigned int) response[5],
        (unsigned int) response[6],
        (unsigned int) response[7]
    );

    protocol_status =
        stn_protocol_parse_result(
            response,
            result
        );

    if (protocol_status != STN_PROTOCOL_OK) {
        stn_log_write(
            "RX RESULT_PARSE_FAILED protocol_status=%d",
            (int) protocol_status
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    stn_log_write(
        "RX RESULT code=%u",
        (unsigned int) *result
    );

    return STN_MINER_OK;
}

stn_miner_status stn_miner_config_default(
    stn_miner_config *config
)
{
    static const char default_host[] =
        "127.0.0.1";

    if (config == NULL) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    memset(
        config,
        0,
        sizeof(*config)
    );

    memcpy(
        config->stratum_host,
        default_host,
        sizeof(default_host)
    );

    config->stratum_port =
        STN_MINER_DEFAULT_PORT;

    return STN_MINER_OK;
}

stn_miner_status stn_miner_run(
    const stn_miner_config *config
)
{
    stn_platform_status platform_status;
    stn_display_state display;

    if (config == NULL ||
        config->address[0] == '\0' ||
        config->stratum_host[0] == '\0' ||
        config->stratum_port == 0u) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    stn_log_write(
        "START address=%s stratum=%s:%u backend=CPU",
        config->address,
        config->stratum_host,
        (unsigned int)
            config->stratum_port
    );

    platform_status =
        stn_platform_init();

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "PLATFORM_INIT_FAILED status=%d",
            (int) platform_status
        );

        return STN_MINER_ERROR;
    }

    stn_log_write(
        "PLATFORM_INIT_OK"
    );

    stn_display_init(
        &display,
        config,
        "CPU"
    );

    stn_display_set_status(
        &display,
        "Starting"
    );

    stn_display_render(
        &display
    );

    for (;;) {
        stn_socket socket;
        stn_miner_status address_status;

        stn_display_set_status(
            &display,
            "Connecting"
        );

        stn_display_render(
            &display
        );

        stn_log_write(
            "CONNECT host=%s port=%u",
            config->stratum_host,
            (unsigned int)
                config->stratum_port
        );

        platform_status =
            stn_platform_socket_connect(
                &socket,
                config->stratum_host,
                config->stratum_port
            );

        if (platform_status !=
            STN_PLATFORM_OK) {

            stn_log_write(
                "CONNECT_FAILED platform_status=%d",
                (int) platform_status
            );

            stn_display_set_status(
                &display,
                "Reconnecting"
            );

            stn_display_set_result(
                &display,
                "Connection failed"
            );

            stn_display_render(
                &display
            );

            stn_platform_sleep_ms(
                STN_MINER_RECONNECT_DELAY_MS
            );

            continue;
        }

        stn_log_write(
            "CONNECT_OK"
        );

        stn_display_set_status(
            &display,
            "Registering address"
        );

        stn_display_render(
            &display
        );

        address_status =
            stn_miner_send_address(
                &socket,
                config->address
            );

        if (address_status !=
            STN_MINER_OK) {

            stn_log_write(
                "ADDRESS_REGISTRATION_FAILED status=%d",
                (int) address_status
            );

            stn_display_set_status(
                &display,
                "Reconnecting"
            );

            stn_display_set_result(
                &display,
                "Address registration failed"
            );

            stn_display_render(
                &display
            );

            stn_platform_socket_close(
                &socket
            );

            stn_log_write(
                "DISCONNECT reason=address_registration_failed"
            );

            stn_platform_sleep_ms(
                STN_MINER_RECONNECT_DELAY_MS
            );

            continue;
        }

        stn_log_write(
            "ADDRESS_REGISTERED address=%s",
            config->address
        );

        stn_display_set_status(
            &display,
            "Waiting for job"
        );

        stn_display_render(
            &display
        );

        for (;;) {
            stn_miner_job job;
            stn_miner_solution solution;
            stn_miner_result_code result;
            stn_miner_status status;
            stn_cpu_status cpu_status;

            uint64_t nonce_start;
            uint64_t hashes_completed;

            memset(
                &job,
                0,
                sizeof(job)
            );

            memset(
                &solution,
                0,
                sizeof(solution)
            );

            stn_log_write(
                "RX WAIT_JOB"
            );

            status =
                stn_miner_receive_job(
                    &socket,
                    &job
                );

            if (status != STN_MINER_OK) {
                stn_log_write(
                    "RX JOB_FAILED miner_status=%d",
                    (int) status
                );

                stn_display_set_status(
                    &display,
                    "Reconnecting"
                );

                stn_display_set_result(
                    &display,
                    "Job receive failed"
                );

                stn_display_render(
                    &display
                );

                stn_miner_job_clear(
                    &job
                );

                break;
            }

            stn_display_set_job(
                &display,
                job.work_id
            );

            stn_display_set_nonce(
                &display,
                job.initial_nonce
            );

            stn_display_set_hashes(
                &display,
                0u
            );

            stn_display_set_status(
                &display,
                "Mining"
            );

            stn_display_render(
                &display
            );

            nonce_start =
                job.initial_nonce;

            hashes_completed = 0u;

            for (;;) {
                stn_log_write(
                    "MINING_BEGIN nonce_start=%llu",
                    (unsigned long long)
                        nonce_start
                );

                cpu_status =
                    stn_cpu_search(
                        &job,
                        nonce_start,
                        UINT64_MAX,
                        &solution
                    );

                if (cpu_status ==
                    STN_CPU_NO_SOLUTION) {

                    stn_log_write(
                        "MINING_NONCE_SPACE_EXHAUSTED"
                    );

                    stn_display_set_status(
                        &display,
                        "Waiting for job"
                    );

                    stn_display_set_result(
                        &display,
                        "Nonce space exhausted"
                    );

                    stn_display_render(
                        &display
                    );

                    break;
                }

                if (cpu_status !=
                    STN_CPU_OK) {

                    stn_log_write(
                        "MINING_CPU_FAILED cpu_status=%d",
                        (int) cpu_status
                    );

                    stn_display_set_status(
                        &display,
                        "Error"
                    );

                    stn_display_set_result(
                        &display,
                        "CPU mining failure"
                    );

                    stn_display_render(
                        &display
                    );

                    stn_miner_job_clear(
                        &job
                    );

                    stn_platform_socket_close(
                        &socket
                    );

                    stn_platform_shutdown();

                    return STN_MINER_ERROR;
                }

                if (solution.nonce >=
                    nonce_start) {

                    hashes_completed +=
                        (solution.nonce -
                         nonce_start) + 1u;
                }

                stn_log_write(
                    "SOLUTION nonce=%llu hashes_completed=%llu",
                    (unsigned long long)
                        solution.nonce,
                    (unsigned long long)
                        hashes_completed
                );

                stn_display_set_nonce(
                    &display,
                    solution.nonce
                );

                stn_display_set_hashes(
                    &display,
                    hashes_completed
                );

                stn_display_set_status(
                    &display,
                    "Submitting"
                );

                stn_display_render(
                    &display
                );

                status =
                    stn_miner_submit_solution(
                        &socket,
                        &solution,
                        &result
                    );

                if (status !=
                    STN_MINER_OK) {

                    stn_log_write(
                        "SUBMIT_OUTCOME_UNCERTAIN miner_status=%d",
                        (int) status
                    );

                    stn_display_set_status(
                        &display,
                        "Reconnecting"
                    );

                    stn_display_set_result(
                        &display,
                        "Submission outcome uncertain"
                    );

                    stn_display_render(
                        &display
                    );

                    stn_miner_job_clear(
                        &job
                    );

                    goto reconnect;
                }

                if (result ==
                    STN_MINER_RESULT_ACCEPTED) {

                    stn_log_write(
                        "RESULT_ACCEPTED nonce=%llu",
                        (unsigned long long)
                            solution.nonce
                    );

                    stn_display_set_status(
                        &display,
                        "Waiting for job"
                    );

                    stn_display_set_result(
                        &display,
                        "Accepted"
                    );

                    stn_display_render(
                        &display
                    );

                    break;
                }

                if (result ==
                    STN_MINER_RESULT_REJECTED) {

                    stn_log_write(
                        "RESULT_REJECTED nonce=%llu",
                        (unsigned long long)
                            solution.nonce
                    );

                    stn_display_set_result(
                        &display,
                        "Chain rejected"
                    );

                    if (solution.nonce ==
                        UINT64_MAX) {

                        stn_display_set_status(
                            &display,
                            "Waiting for job"
                        );

                        stn_display_render(
                            &display
                        );

                        break;
                    }

                    nonce_start =
                        solution.nonce + 1u;

                    stn_display_set_nonce(
                        &display,
                        nonce_start
                    );

                    stn_display_set_status(
                        &display,
                        "Mining"
                    );

                    stn_display_render(
                        &display
                    );

                    continue;
                }

                if (result ==
                    STN_MINER_RESULT_STALE) {

                    stn_log_write(
                        "RESULT_STALE nonce=%llu",
                        (unsigned long long)
                            solution.nonce
                    );

                    stn_display_set_status(
                        &display,
                        "Waiting for job"
                    );

                    stn_display_set_result(
                        &display,
                        "Stale"
                    );

                    stn_display_render(
                        &display
                    );

                    break;
                }

                if (result ==
                    STN_MINER_RESULT_PROVIDER) {

                    stn_log_write(
                        "RESULT_PROVIDER_UNAVAILABLE"
                    );

                    stn_display_set_status(
                        &display,
                        "Waiting for provider"
                    );

                    stn_display_set_result(
                        &display,
                        "Provider unavailable"
                    );

                    stn_display_render(
                        &display
                    );

                    break;
                }

                if (result ==
                    STN_MINER_RESULT_MALFORMED) {

                    stn_log_write(
                        "RESULT_PROTOCOL_REJECTED"
                    );

                    stn_display_set_status(
                        &display,
                        "Reconnecting"
                    );

                    stn_display_set_result(
                        &display,
                        "Protocol rejected"
                    );

                    stn_display_render(
                        &display
                    );

                    stn_miner_job_clear(
                        &job
                    );

                    goto reconnect;
                }

                stn_log_write(
                    "RESULT_UNKNOWN code=%u",
                    (unsigned int) result
                );

                stn_display_set_status(
                    &display,
                    "Reconnecting"
                );

                stn_display_set_result(
                    &display,
                    "Unknown Stratum result"
                );

                stn_display_render(
                    &display
                );

                stn_miner_job_clear(
                    &job
                );

                goto reconnect;
            }

            stn_miner_job_clear(
                &job
            );
        }

reconnect:
        stn_platform_socket_close(
            &socket
        );

        stn_log_write(
            "DISCONNECT reconnecting"
        );

        stn_display_set_status(
            &display,
            "Reconnecting"
        );

        stn_display_render(
            &display
        );

        stn_platform_sleep_ms(
            STN_MINER_RECONNECT_DELAY_MS
        );
    }
}