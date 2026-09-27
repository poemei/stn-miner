#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stn_protocol.h"

int main(void)
{
    uint8_t response[STNM_RESULT_SIZE];
    stn_miner_result_code result;
    stn_protocol_status status;

    memset(response, 0, sizeof(response));
    response[0] = STNM_MAGIC_0;
    response[1] = STNM_MAGIC_1;
    response[2] = STNM_MAGIC_2;
    response[3] = STNM_MAGIC_3;
    response[4] = STNM_VERSION;
    response[5] = STNM_TYPE_RESULT;

    stn_protocol_write_u32_be(
        &response[8],
        (uint32_t) STN_MINER_RESULT_PROVIDER
    );

    status = stn_protocol_parse_result(
        response,
        &result
    );

    if (status != STN_PROTOCOL_OK) {
        return 1;
    }

    if (result != STN_MINER_RESULT_REJECTED) {
        return 1;
    }

    puts("Provider submission result is non-consuming: PASS");
    return 0;
}
