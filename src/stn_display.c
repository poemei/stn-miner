/* stn-miner\src\stn_display.c */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stn_display.h"
#include "stn_platform.h"

static void stn_display_copy_text(
    char *destination,
    size_t destination_size,
    const char *source
)
{
    size_t length;

    if (destination == NULL ||
        destination_size == 0u) {
        return;
    }

    destination[0] = '\0';

    if (source == NULL) {
        return;
    }

    length = strlen(source);

    if (length >= destination_size) {
        length = destination_size - 1u;
    }

    memcpy(
        destination,
        source,
        length
    );

    destination[length] = '\0';
}

static void stn_display_format_job(
    char output[STN_DISPLAY_JOB_TEXT_SIZE],
    const uint8_t work_id[STNM_WORK_ID_SIZE]
)
{
    static const char hex[] =
        "0123456789abcdef";

    size_t i;
    size_t position;

    if (output == NULL) {
        return;
    }

    memset(
        output,
        0,
        STN_DISPLAY_JOB_TEXT_SIZE
    );

    if (work_id == NULL) {
        stn_display_copy_text(
            output,
            STN_DISPLAY_JOB_TEXT_SIZE,
            "-"
        );

        return;
    }

    position = 0u;

    for (i = 0u; i < 4u; ++i) {
        output[position++] =
            hex[(work_id[i] >> 4) & 0x0fu];

        output[position++] =
            hex[work_id[i] & 0x0fu];
    }

    output[position++] = '.';
    output[position++] = '.';
    output[position++] = '.';

    for (i = STNM_WORK_ID_SIZE - 4u;
         i < STNM_WORK_ID_SIZE;
         ++i) {

        output[position++] =
            hex[(work_id[i] >> 4) & 0x0fu];

        output[position++] =
            hex[work_id[i] & 0x0fu];
    }

    output[position] = '\0';
}

static void stn_display_push_job(
    stn_display_state *state,
    const char *job
)
{
    size_t i;
    size_t last;

    if (state == NULL || job == NULL) {
        return;
    }

    if (state->job_count <
        STN_DISPLAY_HISTORY_COUNT) {

        last = state->job_count;
        ++state->job_count;
    } else {
        last =
            STN_DISPLAY_HISTORY_COUNT - 1u;
    }

    for (i = last; i > 0u; --i) {
        state->jobs[i] =
            state->jobs[i - 1u];
    }

    memset(
        &state->jobs[0],
        0,
        sizeof(state->jobs[0])
    );

    stn_display_copy_text(
        state->jobs[0].job,
        sizeof(state->jobs[0].job),
        job
    );

    stn_display_copy_text(
        state->jobs[0].result,
        sizeof(state->jobs[0].result),
        "Mining"
    );

    state->jobs[0].nonce = 0u;
}

void stn_display_init(
    stn_display_state *state,
    const stn_miner_config *config,
    const char *backend
)
{
    if (state == NULL ||
        config == NULL) {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );

    stn_display_copy_text(
        state->address,
        sizeof(state->address),
        config->address
    );

    stn_display_copy_text(
        state->stratum_host,
        sizeof(state->stratum_host),
        config->stratum_host
    );

    state->stratum_port =
        config->stratum_port;

    stn_display_copy_text(
        state->backend,
        sizeof(state->backend),
        backend
    );

    stn_display_copy_text(
        state->gpu,
        sizeof(state->gpu),
        "Not detected"
    );

    stn_display_copy_text(
        state->status,
        sizeof(state->status),
        "Starting"
    );

    stn_display_copy_text(
        state->current_job,
        sizeof(state->current_job),
        "-"
    );

    state->nonce = 0u;
    state->hashes_completed = 0u;
    state->total_shares = 0u;
    state->job_count = 0u;
}

void stn_display_set_gpu(
    stn_display_state *state,
    const char *gpu
)
{
    if (state == NULL) {
        return;
    }

    if (gpu == NULL ||
        gpu[0] == '\0') {

        stn_display_copy_text(
            state->gpu,
            sizeof(state->gpu),
            "Not detected"
        );

        return;
    }

    stn_display_copy_text(
        state->gpu,
        sizeof(state->gpu),
        gpu
    );
}

void stn_display_set_status(
    stn_display_state *state,
    const char *status
)
{
    if (state == NULL) {
        return;
    }

    stn_display_copy_text(
        state->status,
        sizeof(state->status),
        status
    );

    /*
     * A rejected solution can continue mining the
     * same work item. If mining resumes, restore the
     * current history entry to Mining.
     */
    if (status != NULL &&
        strcmp(status, "Mining") == 0 &&
        state->job_count > 0u) {

        stn_display_copy_text(
            state->jobs[0].result,
            sizeof(state->jobs[0].result),
            "Mining"
        );
    }
}

void stn_display_set_job(
    stn_display_state *state,
    const uint8_t work_id[STNM_WORK_ID_SIZE]
)
{
    if (state == NULL) {
        return;
    }

    stn_display_format_job(
        state->current_job,
        work_id
    );

    state->nonce = 0u;
    state->hashes_completed = 0u;

    stn_display_push_job(
        state,
        state->current_job
    );
}

void stn_display_set_nonce(
    stn_display_state *state,
    uint64_t nonce
)
{
    if (state == NULL) {
        return;
    }

    state->nonce = nonce;

    if (state->job_count > 0u) {
        state->jobs[0].nonce = nonce;
    }
}

void stn_display_set_hashes(
    stn_display_state *state,
    uint64_t hashes_completed
)
{
    if (state == NULL) {
        return;
    }

    state->hashes_completed =
        hashes_completed;
}

void stn_display_set_result(
    stn_display_state *state,
    const char *result
)
{
    if (state == NULL ||
        result == NULL ||
        result[0] == '\0') {
        return;
    }

    /*
     * "-" means there is no new terminal result.
     * Do not erase the state of a completed history
     * entry simply because the header is changing.
     */
    if (strcmp(result, "-") == 0) {
        return;
    }

    if (state->job_count == 0u) {
        return;
    }

    /*
     * Only the active Mining entry may receive a new
     * result. This prevents later connection errors
     * from overwriting an already completed job.
     */
    if (strcmp(
            state->jobs[0].result,
            "Mining"
        ) != 0) {
        return;
    }

    stn_display_copy_text(
        state->jobs[0].result,
        sizeof(state->jobs[0].result),
        result
    );
}

void stn_display_add_share(
    stn_display_state *state
)
{
    if (state == NULL) {
        return;
    }

    if (state->total_shares != UINT64_MAX) {
        ++state->total_shares;
    }
}

void stn_display_render(
    const stn_display_state *state
)
{
    size_t i;

    if (state == NULL) {
        return;
    }

    stn_platform_console_clear();

    printf(
        "STN Miner\n"
    );

    printf(
        "Address : %s\n",
        state->address
    );

    printf(
        "Stratum : %s:%u\n",
        state->stratum_host,
        (unsigned int) state->stratum_port
    );

    printf(
        "Backend : %s\n",
        state->backend
    );

    printf(
        "GPU     : %s\n",
        state->gpu
    );

    printf(
        "Status  : %s\n",
        state->status
    );

    printf(
        "Job     : %s\n",
        state->current_job
    );

    printf(
        "Nonce   : %llu\n",
        (unsigned long long) state->nonce
    );

    printf(
        "Hashes  : %llu\n",
        (unsigned long long)
            state->hashes_completed
    );

    printf(
        "Shares  : %llu\n",
        (unsigned long long)
            state->total_shares
    );

    printf(
        "\n"
        "Last 5 Jobs\n"
        "----------------------------------------------------------------\n"
    );

    if (state->job_count == 0u) {
        printf(
            "%-20s %-16s %s\n",
            "-",
            "-",
            "-"
        );
    } else {
        for (i = 0u;
             i < state->job_count;
             ++i) {

            if (state->jobs[i].nonce != 0u) {
                printf(
                    "%-20s %-16s nonce %llu\n",
                    state->jobs[i].job,
                    state->jobs[i].result,
                    (unsigned long long)
                        state->jobs[i].nonce
                );
            } else {
                printf(
                    "%-20s %-16s\n",
                    state->jobs[i].job,
                    state->jobs[i].result
                );
            }
        }
    }

    /*
     * Keep the display height stable while fewer
     * than five jobs have been received.
     */
    for (i = state->job_count;
         i < STN_DISPLAY_HISTORY_COUNT;
         ++i) {

        printf("\n");
    }

    printf(
        "----------------------------------------------------------------\n"
    );

    fflush(stdout);
}