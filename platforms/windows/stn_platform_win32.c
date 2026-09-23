/* C:\poes_projects\stn-miner\platforms\windows\stn_platform_win32.c */

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stn_platform.h"

static SOCKET stn_platform_socket_native(
    const stn_socket *socket
)
{
    if (socket == NULL) {
        return INVALID_SOCKET;
    }

    return (SOCKET) socket->handle;
}

stn_platform_status stn_platform_init(void)
{
    WSADATA data;
    int result;

    memset(
        &data,
        0,
        sizeof(data)
    );

    result = WSAStartup(
        MAKEWORD(2, 2),
        &data
    );

    if (result != 0) {
        return STN_PLATFORM_ERROR;
    }

    return STN_PLATFORM_OK;
}

void stn_platform_shutdown(void)
{
    WSACleanup();
}


stn_platform_status stn_platform_executable_path(
    const char *filename,
    char *output,
    size_t output_size
)
{
    char module_path[STN_PLATFORM_PATH_MAX];
    DWORD length;
    char *separator;
    size_t directory_length;
    size_t filename_length;

    if (filename == NULL ||
        filename[0] == '\0' ||
        output == NULL ||
        output_size == 0u) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    length = GetModuleFileNameA(
        NULL,
        module_path,
        (DWORD) sizeof(module_path)
    );

    if (length == 0u ||
        length >= (DWORD) sizeof(module_path)) {
        return STN_PLATFORM_ERROR;
    }

    module_path[length] = '\0';

    separator = strrchr(
        module_path,
        '\\'
    );

    if (separator == NULL) {
        separator = strrchr(
            module_path,
            '/'
        );
    }

    if (separator == NULL) {
        return STN_PLATFORM_ERROR;
    }

    directory_length =
        (size_t) (separator - module_path) + 1u;

    filename_length =
        strlen(filename);

    if (directory_length +
        filename_length + 1u >
        output_size) {
        return STN_PLATFORM_ERROR;
    }

    memcpy(
        output,
        module_path,
        directory_length
    );

    memcpy(
        output + directory_length,
        filename,
        filename_length
    );

    output[
        directory_length + filename_length
    ] = '\0';

    return STN_PLATFORM_OK;
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

    SOCKET native_socket;
    int result;

    if (connection == NULL ||
        host == NULL ||
        host[0] == '\0') {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    connection->handle =
        (uintptr_t) INVALID_SOCKET;

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

    native_socket = INVALID_SOCKET;

    for (current = addresses;
         current != NULL;
         current = current->ai_next) {

        native_socket = socket(
            current->ai_family,
            current->ai_socktype,
            current->ai_protocol
        );

        if (native_socket == INVALID_SOCKET) {
            continue;
        }

        result = connect(
            native_socket,
            current->ai_addr,
            (int) current->ai_addrlen
        );

        if (result == 0) {
            break;
        }

        closesocket(
            native_socket
        );

        native_socket = INVALID_SOCKET;
    }

    freeaddrinfo(
        addresses
    );

    if (native_socket == INVALID_SOCKET) {
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
    SOCKET native_socket;
    size_t offset;

    if (socket == NULL ||
        data == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket == INVALID_SOCKET) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    offset = 0u;

    while (offset < length) {
        int chunk;
        int sent;

        chunk =
            (length - offset > (size_t) INT_MAX)
            ? INT_MAX
            : (int) (length - offset);

        sent = send(
            native_socket,
            (const char *) (data + offset),
            chunk,
            0
        );

        if (sent == SOCKET_ERROR) {
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
    SOCKET native_socket;
    size_t offset;

    if (socket == NULL ||
        data == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket == INVALID_SOCKET) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    offset = 0u;

    while (offset < length) {
        int chunk;
        int received;

        chunk =
            (length - offset > (size_t) INT_MAX)
            ? INT_MAX
            : (int) (length - offset);

        received = recv(
            native_socket,
            (char *) (data + offset),
            chunk,
            0
        );

        if (received == SOCKET_ERROR) {
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
    SOCKET native_socket;
    int chunk;
    int result;

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

    if (native_socket == INVALID_SOCKET) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    chunk =
        (capacity > (size_t) INT_MAX)
        ? INT_MAX
        : (int) capacity;

    result = recv(
        native_socket,
        (char *) data,
        chunk,
        0
    );

    if (result == SOCKET_ERROR) {
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
    SOCKET native_socket;
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

    if (native_socket == INVALID_SOCKET) {
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

    result = select(
        0,
        &read_set,
        NULL,
        NULL,
        &timeout
    );

    if (result == SOCKET_ERROR) {
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
    SOCKET native_socket;

    if (socket == NULL) {
        return;
    }

    native_socket =
        stn_platform_socket_native(
            socket
        );

    if (native_socket != INVALID_SOCKET) {
        closesocket(
            native_socket
        );
    }

    socket->handle =
        (uintptr_t) INVALID_SOCKET;
}

void stn_platform_sleep_ms(
    uint32_t milliseconds
)
{
    Sleep(
        (DWORD) milliseconds
    );
}

void stn_platform_console_clear(void)
{
    HANDLE output;
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD origin;

    DWORD cells;
    DWORD written;

    output = GetStdHandle(
        STD_OUTPUT_HANDLE
    );

    if (output == NULL ||
        output == INVALID_HANDLE_VALUE) {
        return;
    }

    if (!GetConsoleScreenBufferInfo(
            output,
            &info)) {
        return;
    }

    origin.X = 0;
    origin.Y = 0;

    cells =
        (DWORD) info.dwSize.X *
        (DWORD) info.dwSize.Y;

    written = 0u;

    (void) FillConsoleOutputCharacterA(
        output,
        ' ',
        cells,
        origin,
        &written
    );

    (void) FillConsoleOutputAttribute(
        output,
        info.wAttributes,
        cells,
        origin,
        &written
    );

    (void) SetConsoleCursorPosition(
        output,
        origin
    );
}

stn_platform_status stn_platform_log_append(
    const char *path,
    const uint8_t *data,
    size_t length
)
{
    HANDLE file;
    size_t offset;

    if (path == NULL ||
        path[0] == '\0' ||
        data == NULL) {
        return STN_PLATFORM_INVALID_ARGUMENT;
    }

    file = CreateFileA(
        path,
        FILE_APPEND_DATA,
        FILE_SHARE_READ |
        FILE_SHARE_WRITE |
        FILE_SHARE_DELETE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        return STN_PLATFORM_ERROR;
    }

    offset = 0u;

    while (offset < length) {
        DWORD chunk;
        DWORD written;

        if (length - offset >
            (size_t) MAXDWORD) {

            chunk = MAXDWORD;
        } else {
            chunk =
                (DWORD) (length - offset);
        }

        written = 0u;

        if (!WriteFile(
                file,
                data + offset,
                chunk,
                &written,
                NULL)) {

            CloseHandle(
                file
            );

            return STN_PLATFORM_ERROR;
        }

        if (written == 0u) {
            CloseHandle(
                file
            );

            return STN_PLATFORM_ERROR;
        }

        offset +=
            (size_t) written;
    }

    (void) FlushFileBuffers(
        file
    );

    CloseHandle(
        file
    );

    return STN_PLATFORM_OK;
}