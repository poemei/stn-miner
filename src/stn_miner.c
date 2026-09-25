/* C:\poes_projects\stn-miner\src\stn_miner.c */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stn_backend.h"
#include "stn_compute.h"
#include "stn_cpu.h"
#include "stn_display.h"
#include "stn_gpu.h"
#include "stn_log.h"
#include "stn_miner.h"
#include "stn_platform.h"
#include "stn_protocol.h"

#define STN_MINER_MAX_BLOCK_LENGTH 1051880u
#define STN_MINER_RECONNECT_DELAY_MS 1000u

#define STN_MINER_CPU_HASH_CHUNK 4096u
#define STN_MINER_GPU_HASH_CHUNK 4096u
#define STN_MINER_PROGRESS_INTERVAL_MS 1000u

static uint64_t stn_miner_now_ms(void)
{
    struct timespec current;
    uint64_t seconds;
    uint64_t milliseconds;

    if (timespec_get(
            &current,
            TIME_UTC
        ) != TIME_UTC) {
        return 0u;
    }

    if (current.tv_sec < 0) {
        return 0u;
    }

    seconds =
        (uint64_t) current.tv_sec;

    if (seconds >
        (UINT64_MAX / 1000u)) {
        return UINT64_MAX;
    }

    milliseconds =
        seconds * 1000u;

    milliseconds +=
        (uint64_t)
        (current.tv_nsec / 1000000L);

    return milliseconds;
}

static uint64_t stn_miner_elapsed_ms(
    uint64_t start_ms,
    uint64_t current_ms
)
{
    if (start_ms == 0u ||
        current_ms == 0u ||
        current_ms < start_ms) {
        return 0u;
    }

    return current_ms - start_ms;
}

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

static void stn_miner_detect_compute(
    stn_compute_inventory *inventory
)
{
    stn_compute_status status;
    size_t i;

    if (inventory == NULL) {
        return;
    }

    memset(
        inventory,
        0,
        sizeof(*inventory)
    );

    status =
        stn_compute_detect(
            inventory
        );

    if (status != STN_COMPUTE_OK) {
        stn_log_write(
            "COMPUTE_DETECT_FAILED status=%d",
            (int) status
        );

        return;
    }

    stn_log_write(
        "COMPUTE_DETECT count=%u",
        (unsigned int)
            inventory->count
    );

    for (i = 0u;
         i < inventory->count;
         ++i) {

        stn_log_write(
            "COMPUTE_PROVIDER index=%u type=%s runtime=%s api_ready=%d platform_ready=%d platform_count=%u device_query_ready=%d device_count=%u device_identity_ready=%d device_name=%s device_vendor=%s context_ready=%d queue_ready=%d buffer_ready=%d transfer_ready=%d program_ready=%d build_ready=%d kernel_ready=%d argument_ready=%d execution_ready=%d result_ready=%d vector_ready=%d sha256_ready=%d",
            (unsigned int) i,
            stn_compute_provider_name(
                inventory->providers[i].type
            ),
            inventory->providers[i].runtime,
            inventory->providers[i].api_ready,
            inventory->providers[i].platform_ready,
            (unsigned int)
                inventory->providers[i].platform_count,
            inventory->providers[i].device_query_ready,
            (unsigned int)
                inventory->providers[i].device_count,
            inventory->providers[i].device_identity_ready,
            inventory->providers[i].device_name[0] != '\0'
                ? inventory->providers[i].device_name
                : "-",
            inventory->providers[i].device_vendor[0] != '\0'
                ? inventory->providers[i].device_vendor
                : "-",
            inventory->providers[i].context_ready,
            inventory->providers[i].queue_ready,
            inventory->providers[i].buffer_ready,
            inventory->providers[i].transfer_ready,
            inventory->providers[i].program_ready,
            inventory->providers[i].build_ready,
            inventory->providers[i].kernel_ready,
            inventory->providers[i].argument_ready,
            inventory->providers[i].execution_ready,
            inventory->providers[i].result_ready,
            inventory->providers[i].vector_ready,
            inventory->providers[i].sha256_ready
        );
    }
}

static void stn_miner_detect_gpu(
    stn_display_state *display,
    stn_gpu_inventory *inventory
)
{
    stn_gpu_status gpu_status;

    char gpu_text[STN_DISPLAY_GPU_TEXT_SIZE];

    size_t i;

    if (display == NULL ||
        inventory == NULL) {
        return;
    }

    memset(
        inventory,
        0,
        sizeof(*inventory)
    );

    gpu_status =
        stn_gpu_detect(
            inventory
        );

    if (gpu_status ==
        STN_GPU_NOT_FOUND) {

        stn_log_write(
            "GPU_DETECT none"
        );

        stn_display_set_gpu(
            display,
            "None detected"
        );

        return;
    }

    if (gpu_status !=
        STN_GPU_OK) {

        stn_log_write(
            "GPU_DETECT_FAILED status=%d",
            (int) gpu_status
        );

        stn_display_set_gpu(
            display,
            "Detection unavailable"
        );

        return;
    }

    if (inventory->count == 0u) {
        stn_log_write(
            "GPU_DETECT none"
        );

        stn_display_set_gpu(
            display,
            "None detected"
        );

        return;
    }

    stn_log_write(
        "GPU_DETECT count=%u",
        (unsigned int)
            inventory->count
    );

    for (i = 0u;
         i < inventory->count;
         ++i) {

        stn_log_write(
            "GPU_DEVICE index=%u vendor=%s name=%s",
            (unsigned int)
                inventory->devices[i].device_index,
            stn_gpu_vendor_name(
                inventory->devices[i].vendor
            ),
            inventory->devices[i].name
        );
    }

    if (inventory->count == 1u) {
        (void) snprintf(
            gpu_text,
            sizeof(gpu_text),
            "%s",
            inventory->devices[0].name
        );
    } else {
        (void) snprintf(
            gpu_text,
            sizeof(gpu_text),
            "%s (+%u more)",
            inventory->devices[0].name,
            (unsigned int)
                (inventory->count - 1u)
        );
    }

    gpu_text[
        sizeof(gpu_text) - 1u
    ] = '\0';

    stn_display_set_gpu(
        display,
        gpu_text
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

static stn_miner_status stn_miner_send_hash_progress(
    stn_socket *socket,
    const uint8_t work_id[STNM_WORK_ID_SIZE],
    uint64_t hashes_completed,
    uint64_t elapsed_ms
)
{
    stn_miner_hash_progress progress;
    uint8_t frame[STNM_HASH_PROGRESS_SIZE];

    stn_protocol_status protocol_status;
    stn_platform_status platform_status;

    if (socket == NULL ||
        work_id == NULL) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    if (elapsed_ms == 0u) {
        return STN_MINER_OK;
    }

    memset(
        &progress,
        0,
        sizeof(progress)
    );

    memcpy(
        progress.work_id,
        work_id,
        STNM_WORK_ID_SIZE
    );

    progress.hashes_completed =
        hashes_completed;

    progress.elapsed_ms =
        elapsed_ms;

    protocol_status =
        stn_protocol_build_hash_progress(
            &progress,
            frame
        );

    if (protocol_status != STN_PROTOCOL_OK) {
        stn_log_write(
            "TX HASH_PROGRESS_BUILD_FAILED protocol_status=%d",
            (int) protocol_status
        );

        return STN_MINER_PROTOCOL_ERROR;
    }

    platform_status =
        stn_platform_socket_send_all(
            socket,
            frame,
            sizeof(frame)
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "TX HASH_PROGRESS_FAILED platform_status=%d",
            (int) platform_status
        );

        return STN_MINER_IO_FAILED;
    }

    stn_log_write(
        "TX HASH_PROGRESS hashes=%llu elapsed_ms=%llu",
        (unsigned long long)
            hashes_completed,
        (unsigned long long)
            elapsed_ms
    );

    return STN_MINER_OK;
}

static stn_miner_status stn_miner_socket_readable(
    stn_socket *socket,
    int *readable
)
{
    stn_platform_status platform_status;

    if (socket == NULL ||
        readable == NULL) {
        return STN_MINER_INVALID_ARGUMENT;
    }

    platform_status =
        stn_platform_socket_readable(
            socket,
            readable
        );

    if (platform_status != STN_PLATFORM_OK) {
        stn_log_write(
            "SOCKET_READABLE_FAILED platform_status=%d",
            (int) platform_status
        );

        return STN_MINER_IO_FAILED;
    }

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
            job->chain_target,
            &job->block[
                STNM_BLOCK_TARGET_OFFSET
            ],
            STNM_TARGET_SIZE
        ) != 0) {

        stn_log_write(
            "RX JOB_CHAIN_TARGET_MISMATCH"
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
    stn_backend backend;
    stn_backend_status backend_status;
    stn_backend_type candidate_type;
    stn_gpu_inventory gpu_inventory;
    stn_compute_inventory compute_inventory;
    stn_display_state display;

    if (config == NULL ||
        config->address[0] == '\0' ||
        config->stratum_host[0] == '\0' ||
        config->stratum_port == 0u) {
        return STN_MINER_INVALID_ARGUMENT;
    }

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

    stn_miner_detect_compute(
        &compute_inventory
    );

    backend_status =
        stn_backend_select(
            &backend,
            &compute_inventory
        );

    if (backend_status !=
        STN_BACKEND_OK) {

        stn_log_write(
            "BACKEND_SELECT_FAILED status=%d",
            (int) backend_status
        );

        stn_platform_shutdown();
        return STN_MINER_ERROR;
    }

    stn_log_write(
        "START address=%s stratum=%s:%u backend=%s",
        config->address,
        config->stratum_host,
        (unsigned int)
            config->stratum_port,
        backend.name
    );

    stn_display_init(
        &display,
        config,
        backend.name
    );

    stn_miner_detect_gpu(
        &display,
        &gpu_inventory
    );

    candidate_type =
        stn_backend_candidate_type(
            &gpu_inventory
        );

    stn_log_write(
        "BACKEND_CANDIDATE preferred=%s active=%s",
        stn_backend_type_name(
            candidate_type
        ),
        backend.name
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
        int replacement_pending;

        /*
         * Clear session-visible work before opening a new transport.  A
         * reconnect must never display the previous session's Job/Nonce/
         * Hashes beside the transient Connecting state.
         */
        stn_display_clear_job(
            &display
        );

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

        /*
         * The transport is established at this point.  Do not leave the
         * display carrying the outer-loop "Connecting" state while the live
         * session registers its identity and receives work.
         */
        stn_display_set_status(
            &display,
            "Connected"
        );

        stn_display_render(
            &display
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
            "ADDRESS_SENT address=%s",
            config->address
        );

        replacement_pending = 0;

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

            uint64_t job_start_ms;
            uint64_t last_progress_ms;

            int replaced;

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

            /*
             * Socket readability only proves transport data is available.
             * Confirm a complete, valid JOB before recording the previous
             * work item as Replaced.  EOF or malformed inbound data must not
             * be reported as legitimate replacement work.
             */
            if (replacement_pending) {
                stn_log_write(
                    "RX JOB_REPLACEMENT_CONFIRMED"
                );

                stn_display_set_result(
                    &display,
                    "Replaced"
                );

                replacement_pending = 0;
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

            hashes_completed =
                0u;

            job_start_ms =
                stn_miner_now_ms();

            last_progress_ms =
                job_start_ms;

            replaced = 0;

            stn_log_write(
                "MINING_BEGIN backend=%s nonce_start=%llu chunk=%u",
                backend.name,
                (unsigned long long)
                    nonce_start,
                (unsigned int)
                    (backend.type == STN_BACKEND_TYPE_GPU
                        ? STN_MINER_GPU_HASH_CHUNK
                        : STN_MINER_CPU_HASH_CHUNK)
            );

            for (;;) {
                uint64_t chunk_end;
                uint64_t hash_chunk;
                uint64_t chunk_hashes;
                uint64_t current_ms;
                uint64_t elapsed_ms;

                int readable;

                readable = 0;

                status =
                    stn_miner_socket_readable(
                        &socket,
                        &readable
                    );

                if (status != STN_MINER_OK) {
                    stn_display_set_status(
                        &display,
                        "Reconnecting"
                    );

                    stn_display_set_result(
                        &display,
                        "Stratum read failure"
                    );

                    stn_display_render(
                        &display
                    );

                    stn_miner_job_clear(
                        &job
                    );

                    goto reconnect;
                }

                if (readable) {
                    stn_log_write(
                        "RX JOB_PENDING while_mining=1"
                    );

                    replacement_pending = 1;

                    stn_display_set_status(
                        &display,
                        "Waiting for job"
                    );

                    stn_display_render(
                        &display
                    );

                    replaced = 1;
                    break;
                }

                hash_chunk =
                    backend.type == STN_BACKEND_TYPE_GPU
                        ? STN_MINER_GPU_HASH_CHUNK
                        : STN_MINER_CPU_HASH_CHUNK;

                if (nonce_start >
                    UINT64_MAX -
                    (hash_chunk - 1u)) {

                    chunk_end =
                        UINT64_MAX;
                } else {
                    chunk_end =
                        nonce_start +
                        (hash_chunk - 1u);
                }

                backend_status =
                    stn_backend_search(
                        &backend,
                        &job,
                        nonce_start,
                        chunk_end,
                        &solution
                    );

                if (backend_status ==
                    STN_BACKEND_NO_SOLUTION) {

                    chunk_hashes =
                        (chunk_end -
                         nonce_start) + 1u;

                    if (UINT64_MAX -
                        hashes_completed <
                        chunk_hashes) {

                        hashes_completed =
                            UINT64_MAX;
                    } else {
                        hashes_completed +=
                            chunk_hashes;
                    }

                    stn_display_set_nonce(
                        &display,
                        chunk_end
                    );

                    stn_display_set_hashes(
                        &display,
                        hashes_completed
                    );

                    current_ms =
                        stn_miner_now_ms();

                    elapsed_ms =
                        stn_miner_elapsed_ms(
                            job_start_ms,
                            current_ms
                        );

                    if (current_ms != 0u &&
                        (last_progress_ms == 0u ||
                         current_ms <
                            last_progress_ms ||
                         current_ms -
                            last_progress_ms >=
                            STN_MINER_PROGRESS_INTERVAL_MS)) {

                        status =
                            stn_miner_send_hash_progress(
                                &socket,
                                job.work_id,
                                hashes_completed,
                                elapsed_ms
                            );

                        if (status !=
                            STN_MINER_OK) {

                            stn_display_set_status(
                                &display,
                                "Reconnecting"
                            );

                            stn_display_set_result(
                                &display,
                                "Progress send failed"
                            );

                            stn_display_render(
                                &display
                            );

                            stn_miner_job_clear(
                                &job
                            );

                            goto reconnect;
                        }

                        last_progress_ms =
                            current_ms;

                        stn_display_render(
                            &display
                        );
                    }

                    readable = 0;

                    status =
                        stn_miner_socket_readable(
                            &socket,
                            &readable
                        );

                    if (status !=
                        STN_MINER_OK) {

                        stn_display_set_status(
                            &display,
                            "Reconnecting"
                        );

                        stn_display_set_result(
                            &display,
                            "Stratum read failure"
                        );

                        stn_display_render(
                            &display
                        );

                        stn_miner_job_clear(
                            &job
                        );

                        goto reconnect;
                    }

                    if (readable) {
                        stn_log_write(
                            "RX JOB_PENDING after_chunk=1"
                        );

                        replacement_pending = 1;

                        stn_display_set_status(
                            &display,
                            "Waiting for job"
                        );

                        stn_display_render(
                            &display
                        );

                        replaced = 1;
                        break;
                    }

                    if (chunk_end ==
                        UINT64_MAX) {

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

                    nonce_start =
                        chunk_end + 1u;

                    continue;
                }

                if (backend_status !=
                    STN_BACKEND_OK) {

                    stn_log_write(
                        "MINING_BACKEND_FAILED backend=%s status=%d",
                        backend.name,
                        (int) backend_status
                    );

                    stn_display_set_status(
                        &display,
                        "Error"
                    );

                    stn_display_set_result(
                        &display,
                        "Backend mining failure"
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

                chunk_hashes =
                    (solution.nonce -
                     nonce_start) + 1u;

                if (UINT64_MAX -
                    hashes_completed <
                    chunk_hashes) {

                    hashes_completed =
                        UINT64_MAX;
                } else {
                    hashes_completed +=
                        chunk_hashes;
                }

                current_ms =
                    stn_miner_now_ms();

                elapsed_ms =
                    stn_miner_elapsed_ms(
                        job_start_ms,
                        current_ms
                    );

                stn_log_write(
                    "SOLUTION nonce=%llu hashes_completed=%llu elapsed_ms=%llu",
                    (unsigned long long)
                        solution.nonce,
                    (unsigned long long)
                        hashes_completed,
                    (unsigned long long)
                        elapsed_ms
                );

                stn_display_set_nonce(
                    &display,
                    solution.nonce
                );

                stn_display_set_hashes(
                    &display,
                    hashes_completed
                );

                readable = 0;

                status =
                    stn_miner_socket_readable(
                        &socket,
                        &readable
                    );

                if (status !=
                    STN_MINER_OK) {

                    stn_display_set_status(
                        &display,
                        "Reconnecting"
                    );

                    stn_display_set_result(
                        &display,
                        "Stratum read failure"
                    );

                    stn_display_render(
                        &display
                    );

                    stn_miner_job_clear(
                        &job
                    );

                    goto reconnect;
                }

                if (readable) {
                    stn_log_write(
                        "RX JOB_PENDING before_submit=1"
                    );

                    replacement_pending = 1;

                    stn_display_set_status(
                        &display,
                        "Waiting for job"
                    );

                    stn_display_render(
                        &display
                    );

                    replaced = 1;
                    break;
                }

                status =
                    stn_miner_send_hash_progress(
                        &socket,
                        job.work_id,
                        hashes_completed,
                        elapsed_ms
                    );

                if (status !=
                    STN_MINER_OK) {

                    stn_display_set_status(
                        &display,
                        "Reconnecting"
                    );

                    stn_display_set_result(
                        &display,
                        "Progress send failed"
                    );

                    stn_display_render(
                        &display
                    );

                    stn_miner_job_clear(
                        &job
                    );

                    goto reconnect;
                }

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

                    stn_display_add_share(
                        &display
                    );

                    stn_display_set_result(
                        &display,
                        "Accepted"
                    );

                    /*
                     * Phase 19 qualifying-share acceptance does not consume
                     * the active Work ID. Continue from the next nonce until
                     * Stratum explicitly replaces or stales the job.
                     */
                    if (solution.nonce == UINT64_MAX) {
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

            if (replaced) {
                continue;
            }
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