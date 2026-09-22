/* stn-miner\platforms\linux\stn_platform_linux.c */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "stn_platform.h"

static int stn_platform_socket_native(
    const stn_socket *socket
)
{
    if (socket == NULL) {
        return -1;
    }

    return (int) socket->handle;
}

stn_platform_status stn_platform_init(void)
{
    return STN_PLATFORM_OK;
}

void stn_platform_shutdown(void)
{
}

stn_platform_status stn_platform_socket_connect(
    stn_socket *connection,
    const char *host,
    uint16_t port
)
{
    struct addrinfo hints;
    struct addrinfo *addresses;
    struct addrinfo *current;

    char service[16];

    int native_socket;
    int result;

    if (connection == NULL ||
        host == NULL ||
        host[0] == '\0') {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    connection->handle =
        (uintptr_t) -1;

    memset(
        &hints,
        0,
        sizeof(hints)
    );

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = snprintf(
        service,
        sizeof(service),
        "%u",
        (unsigned int) port
    );

    if (result < 0 ||
        (size_t) result >= sizeof(service)) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    addresses = NULL;

    result = getaddrinfo(
        host,
        service,
        &hints,
        &addresses
    );

    if (result != 0 ||
        addresses == NULL) {
        return STN_PLATFORM_CONNECT_FAILED;
    }

    native_socket = -1;

    for (current = addresses;
         current != NULL;
         current = current->ai_next) {

        native_socket = socket(
            current->ai_family,
            current->ai_socktype,
            current->ai_protocol
        );

        if (native_socket < 0) {
            continue;
        }

        result = connect(
            native_socket,
            current->ai_addr,
            current->ai_addrlen
        );

        if (result == 0) {
            break;
        }

        close(
            native_socket
        );

        native_socket = -1;
    }

    freeaddrinfo(
        addresses
    );

    if (native_socket < 0) {
        return STN_PLATFORM_CONNECT_FAILED;
    }

    connection->handle =
        (uintptr_t) native_socket;

    return STN_PLATFORM_OK;
}

stn_platform_status stn_platform_socket_send_all(
    stn_socket *socket,
    const uint8_t *data,
    size_t length
)
{
    int native_socket;
    size_t offset;

    if (socket == NULL ||
        data == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket < 0) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    offset = 0u;

    while (offset < length) {
        ssize_t sent;

        sent = send(
            native_socket,
            data + offset,
            length - offset,
            0
        );

        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }

            return STN_PLATFORM_SEND_FAILED;
        }

        if (sent == 0) {
            return STN_PLATFORM_CLOSED;
        }

        offset +=
            (size_t) sent;
    }

    return STN_PLATFORM_OK;
}

stn_platform_status stn_platform_socket_receive_all(
    stn_socket *socket,
    uint8_t *data,
    size_t length
)
{
    int native_socket;
    size_t offset;

    if (socket == NULL ||
        data == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket < 0) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    offset = 0u;

    while (offset < length) {
        ssize_t received;

        received = recv(
            native_socket,
            data + offset,
            length - offset,
            0
        );

        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }

            return STN_PLATFORM_RECEIVE_FAILED;
        }

        if (received == 0) {
            return STN_PLATFORM_CLOSED;
        }

        offset +=
            (size_t) received;
    }

    return STN_PLATFORM_OK;
}

stn_platform_status stn_platform_socket_receive_some(
    stn_socket *socket,
    uint8_t *data,
    size_t capacity,
    size_t *received
)
{
    int native_socket;
    ssize_t result;

    if (socket == NULL ||
        data == NULL ||
        received == NULL ||
        capacity == 0u) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket < 0) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    for (;;) {
        result = recv(
            native_socket,
            data,
            capacity,
            0
        );

        if (result < 0 &&
            errno == EINTR) {
            continue;
        }

        break;
    }

    if (result < 0) {
        *received = 0u;
        return STN_PLATFORM_RECEIVE_FAILED;
    }

    if (result == 0) {
        *received = 0u;
        return STN_PLATFORM_CLOSED;
    }

    *received =
        (size_t) result;

    return STN_PLATFORM_OK;
}

stn_platform_status stn_platform_socket_readable(
    stn_socket *socket,
    int *readable
)
{
    int native_socket;
    fd_set read_set;
    struct timeval timeout;
    int result;

    if (socket == NULL ||
        readable == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket < 0) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    *readable = 0;

    FD_ZERO(
        &read_set
    );

    FD_SET(
        native_socket,
        &read_set
    );

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    for (;;) {
        result = select(
            native_socket + 1,
            &read_set,
            NULL,
            NULL,
            &timeout
        );

        if (result < 0 &&
            errno == EINTR) {

            FD_ZERO(
                &read_set
            );

            FD_SET(
                native_socket,
                &read_set
            );

            timeout.tv_sec = 0;
            timeout.tv_usec = 0;

            continue;
        }

        break;
    }

    if (result < 0) {
        return STN_PLATFORM_RECEIVE_FAILED;
    }

    if (result > 0 &&
        FD_ISSET(
            native_socket,
            &read_set
        )) {

        *readable = 1;
    }

    return STN_PLATFORM_OK;
}

void stn_platform_socket_close(
    stn_socket *socket
)
{
    int native_socket;

    if (socket == NULL) {
        return;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket >= 0) {
        close(
            native_socket
        );
    }

    socket->handle =
        (uintptr_t) -1;
}

void stn_platform_sleep_ms(
    uint32_t milliseconds
)
{
    struct timespec delay;

    delay.tv_sec =
        (time_t) (milliseconds / 1000u);

    delay.tv_nsec =
        (long)
        ((milliseconds % 1000u) *
         1000000u);

    while (nanosleep(
            &delay,
            &delay
        ) != 0) {

        if (errno != EINTR) {
            break;
        }
    }
}

void stn_platform_console_clear(void)
{
    static const char clear_screen[] =
        "\033[2J\033[H";

    (void) fwrite(
        clear_screen,
        1u,
        sizeof(clear_screen) - 1u,
        stdout
    );

    (void) fflush(
        stdout
    );
}

stn_platform_status stn_platform_log_append(
    const char *path,
    const uint8_t *data,
    size_t length
)
{
    int file;
    size_t offset;

    if (path == NULL ||
        path[0] == '\0' ||
        data == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    file = open(
        path,
        O_WRONLY |
        O_CREAT |
        O_APPEND,
        0644
    );

    if (file < 0) {
        return STN_PLATFORM_ERROR;
    }

    offset = 0u;

    while (offset < length) {
        ssize_t written;

        written = write(
            file,
            data + offset,
            length - offset
        );

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }

            close(
                file
            );

            return STN_PLATFORM_ERROR;
        }

        if (written == 0) {
            close(
                file
            );

            return STN_PLATFORM_ERROR;
        }

        offset +=
            (size_t) written;
    }

    (void) fsync(
        file
    );

    close(
        file
    );

    return STN_PLATFORM_OK;
}