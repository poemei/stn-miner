/* stn-miner\includes\stn_display.h */

#ifndef STN_DISPLAY_H
#define STN_DISPLAY_H

#include <stddef.h>
#include <stdint.h>

#include "stn_miner.h"

#define STN_DISPLAY_JOB_TEXT_SIZE 20u
#define STN_DISPLAY_RESULT_TEXT_SIZE 32u
#define STN_DISPLAY_STATUS_TEXT_SIZE 32u
#define STN_DISPLAY_BACKEND_TEXT_SIZE 32u

#define STN_DISPLAY_HISTORY_COUNT 5u

typedef struct stn_display_job_entry {
    char job[STN_DISPLAY_JOB_TEXT_SIZE];
    char result[STN_DISPLAY_RESULT_TEXT_SIZE];
    uint64_t nonce;
} stn_display_job_entry;

typedef struct stn_display_state {
    char address[STN_MINER_ADDRESS_SIZE];
    char stratum_host[STN_MINER_HOST_MAX];
    uint16_t stratum_port;

    char backend[STN_DISPLAY_BACKEND_TEXT_SIZE];
    char status[STN_DISPLAY_STATUS_TEXT_SIZE];

    char current_job[STN_DISPLAY_JOB_TEXT_SIZE];

    uint64_t nonce;
    uint64_t hashes_completed;

    stn_display_job_entry jobs[STN_DISPLAY_HISTORY_COUNT];
    size_t job_count;
} stn_display_state;

void stn_display_init(
    stn_display_state *state,
    const stn_miner_config *config,
    const char *backend
);

void stn_display_set_status(
    stn_display_state *state,
    const char *status
);

void stn_display_set_job(
    stn_display_state *state,
    const uint8_t work_id[STNM_WORK_ID_SIZE]
);

void stn_display_set_nonce(
    stn_display_state *state,
    uint64_t nonce
);

void stn_display_set_hashes(
    stn_display_state *state,
    uint64_t hashes_completed
);

void stn_display_set_result(
    stn_display_state *state,
    const char *result
);

void stn_display_render(
    const stn_display_state *state
);

#endif