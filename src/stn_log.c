/* stn-miner\src\stn_log.c */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "stn_log.h"
#include "stn_platform.h"

#define STN_LOG_MESSAGE_SIZE 2048u

void stn_log_write(
    const char *format,
    ...
)
{
    char message[STN_LOG_MESSAGE_SIZE];
    char line[STN_LOG_MESSAGE_SIZE + 64u];

    time_t now;
    struct tm *local_time;

    va_list arguments;

    int message_length;
    int line_length;

    if (format == NULL) {
        return;
    }

    va_start(
        arguments,
        format
    );

    message_length = vsnprintf(
        message,
        sizeof(message),
        format,
        arguments
    );

    va_end(arguments);

    if (message_length < 0) {
        return;
    }

    message[
        sizeof(message) - 1u
    ] = '\0';

    now = time(NULL);

    local_time = localtime(
        &now
    );

    if (local_time == NULL) {
        return;
    }

    line_length = snprintf(
        line,
        sizeof(line),
        "[%04d-%02d-%02d %02d:%02d:%02d] %s\r\n",
        local_time->tm_year + 1900,
        local_time->tm_mon + 1,
        local_time->tm_mday,
        local_time->tm_hour,
        local_time->tm_min,
        local_time->tm_sec,
        message
    );

    if (line_length <= 0) {
        return;
    }

    if ((size_t) line_length >= sizeof(line)) {
        line_length =
            (int) sizeof(line) - 1;
    }

    stn_platform_log_append(
        STN_LOG_FILENAME,
        (const uint8_t *) line,
        (size_t) line_length
    );
}