/* C:\poes_projects\stn-miner\includes\stn_cpu.h */

#ifndef STN_CPU_H
#define STN_CPU_H

#include <stdint.h>

#include "stn_miner.h"

typedef enum stn_cpu_status {
    STN_CPU_OK = 0,
    STN_CPU_INVALID_ARGUMENT = 1,
    STN_CPU_NO_SOLUTION = 2
} stn_cpu_status;

stn_cpu_status stn_cpu_search(
    stn_miner_job *job,
    uint64_t nonce_start,
    uint64_t nonce_end,
    stn_miner_solution *solution
);

#endif