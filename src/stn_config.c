/* stn-miner\src\stn_config.c */

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stn_config.h"

static const char *stn_config_skip_space(
    const char *text
)
{
    if (text == NULL) {
        return NULL;
    }

    while (*text != '\0' &&
           isspace((unsigned char) *text)) {
        ++text;
    }

    return text;
}

static const char *stn_config_find_key(
    const char *json,
    const char *key
)
{
    char pattern[64];
    int written;
    const char *position;

    if (json == NULL || key == NULL) {
        return NULL;
    }

    written = snprintf(
        pattern,
        sizeof(pattern),
        "\"%s\"",
        key
    );

    if (written < 0 ||
        (size_t) written >= sizeof(pattern)) {
        return NULL;
    }

    position = strstr(
        json,
        pattern
    );

    if (position == NULL) {
        return NULL;
    }

    position += strlen(pattern);
    position = stn_config_skip_space(position);

    if (position == NULL || *position != ':') {
        return NULL;
    }

    ++position;

    return stn_config_skip_space(position);
}

static int stn_config_read_string(
    const char *json,
    const char *key,
    char *output,
    size_t output_size
)
{
    const char *position;
    size_t length;

    if (json == NULL ||
        key == NULL ||
        output == NULL ||
        output_size == 0u) {
        return 0;
    }

    position = stn_config_find_key(
        json,
        key
    );

    if (position == NULL || *position != '"') {
        return 0;
    }

    ++position;
    length = 0u;

    while (position[length] != '\0' &&
           position[length] != '"') {

        if (position[length] == '\\') {
            return 0;
        }

        if (length + 1u >= output_size) {
            return 0;
        }

        output[length] = position[length];
        ++length;
    }

    if (position[length] != '"') {
        return 0;
    }

    output[length] = '\0';

    return 1;
}

static int stn_config_read_port(
    const char *json,
    uint16_t *port
)
{
    const char *position;
    unsigned long value;
    char *end;

    if (json == NULL || port == NULL) {
        return 0;
    }

    position = stn_config_find_key(
        json,
        "port"
    );

    if (position == NULL) {
        return 0;
    }

    value = strtoul(
        position,
        &end,
        10
    );

    if (end == position) {
        return 0;
    }

    end = (char *) stn_config_skip_space(end);

    if (*end != ',' &&
        *end != '}' &&
        *end != '\0') {
        return 0;
    }

    if (value == 0ul ||
        value > 65535ul) {
        return 0;
    }

    *port = (uint16_t) value;

    return 1;
}

static int stn_config_valid_address(
    const char *address
)
{
    size_t i;

    if (address == NULL) {
        return 0;
    }

    if (strlen(address) != STN_MINER_ADDRESS_LENGTH) {
        return 0;
    }

    if (memcmp(
            address,
            "stn0_",
            5u
        ) != 0) {
        return 0;
    }

    for (i = 5u; i < STN_MINER_ADDRESS_LENGTH; ++i) {
        char c;

        c = address[i];

        if (!((c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f'))) {
            return 0;
        }
    }

    return 1;
}

stn_config_status stn_config_load(
    const char *path,
    stn_miner_config *config
)
{
    FILE *file;
    char *json;
    long file_size;
    size_t bytes_read;

    if (path == NULL || config == NULL) {
        return STN_CONFIG_INVALID_ARGUMENT;
    }

    memset(
        config,
        0,
        sizeof(*config)
    );

    file = fopen(
        path,
        "rb"
    );

    if (file == NULL) {
        return STN_CONFIG_OPEN_FAILED;
    }

    if (fseek(
            file,
            0,
            SEEK_END
        ) != 0) {

        fclose(file);
        return STN_CONFIG_READ_FAILED;
    }

    file_size = ftell(file);

    if (file_size <= 0 ||
        (unsigned long) file_size >
            STN_CONFIG_MAX_FILE_SIZE) {

        fclose(file);
        return STN_CONFIG_READ_FAILED;
    }

    if (fseek(
            file,
            0,
            SEEK_SET
        ) != 0) {

        fclose(file);
        return STN_CONFIG_READ_FAILED;
    }

    json = (char *) malloc(
        (size_t) file_size + 1u
    );

    if (json == NULL) {
        fclose(file);
        return STN_CONFIG_READ_FAILED;
    }

    bytes_read = fread(
        json,
        1u,
        (size_t) file_size,
        file
    );

    fclose(file);

    if (bytes_read != (size_t) file_size) {
        free(json);
        return STN_CONFIG_READ_FAILED;
    }

    json[bytes_read] = '\0';

    if (!stn_config_read_string(
            json,
            "address",
            config->address,
            sizeof(config->address))) {

        free(json);
        return STN_CONFIG_INVALID_JSON;
    }

    if (!stn_config_valid_address(
            config->address)) {

        free(json);
        return STN_CONFIG_INVALID_ADDRESS;
    }

    if (!stn_config_read_string(
            json,
            "host",
            config->stratum_host,
            sizeof(config->stratum_host))) {

        free(json);
        return STN_CONFIG_INVALID_HOST;
    }

    if (config->stratum_host[0] == '\0') {
        free(json);
        return STN_CONFIG_INVALID_HOST;
    }

    if (!stn_config_read_port(
            json,
            &config->stratum_port)) {

        free(json);
        return STN_CONFIG_INVALID_PORT;
    }

    free(json);

    return STN_CONFIG_OK;
}