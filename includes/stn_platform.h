/* C:\poes_projects\stn-miner\includes\stn_platform.h */

#ifndef STN_PLATFORM_H
#define STN_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

#define STN_PLATFORM_PATH_MAX 1024u

typedef struct stn_socket {
    uintptr_t handle;
} stn_socket;

typedef enum stn_platform_status {
    STN_PLATFORM_OK = 0,
    STN_PLATFORM_ERROR = 1,
    STN_PLATFORM_INVALID_ARGUMENT = 2,
    STN_PLATFORM_CONNECT_FAILED = 3,
    STN_PLATFORM_SEND_FAILED = 4,
    STN_PLATFORM_RECEIVE_FAILED = 5,
    STN_PLATFORM_CLOSED = 6
} stn_platform_status;

stn_platform_status stn_platform_init(void);

stn_platform_status stn_platform_executable_path(
    const char *filename,
    char *output,
    size_t output_size
);

void stn_platform_shutdown(void);

stn_platform_status stn_platform_socket_connect(
    stn_socket *socket,
    const char *host,
    uint16_t port
);

stn_platform_status stn_platform_socket_send_all(
    stn_socket *socket,
    const uint8_t *data,
    size_t length
);

stn_platform_status stn_platform_socket_receive_all(
    stn_socket *socket,
    uint8_t *data,
    size_t length
);

stn_platform_status stn_platform_socket_receive_some(
    stn_socket *socket,
    uint8_t *data,
    size_t capacity,
    size_t *received
);

stn_platform_status stn_platform_socket_readable(
    stn_socket *socket,
    int *readable
);

void stn_platform_socket_close(
    stn_socket *socket
);

void stn_platform_sleep_ms(
    uint32_t milliseconds
);

void stn_platform_console_clear(void);

stn_platform_status stn_platform_log_append(
    const char *path,
    const uint8_t *data,
    size_t length
);

#endif