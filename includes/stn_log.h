/* stn-miner\includes\stn_log.h */

#ifndef STN_LOG_H
#define STN_LOG_H

#define STN_LOG_FILENAME "stn-miner.log"

void stn_log_write(
    const char *format,
    ...
);

#endif