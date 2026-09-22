#include "telemetry.h"
#include "uart_driver.h"

static bool transmission_active = false;

void telemetry_init(void)
{
    transmission_active = false;
}

bool telemetry_encode(uint8_t sensor_id, uint16_t reading,
                      uint8_t *output, size_t output_capacity)
{
    /* TODO 1: Implement the packet contract in include/telemetry.h. */
    (void)sensor_id;
    (void)reading;
    (void)output;
    (void)output_capacity;
    return false;
}

telemetry_result_t telemetry_send(uint8_t sensor_id, uint16_t reading)
{
    /* TODO 2: Encode and start a transfer; manage buffer lifetime and busy
     * state. Choose suitable storage for the bytes. Use transmission_active
     * and the supplied callback below. Handle a refused driver start.
     */
    (void)sensor_id;
    (void)reading;
    return TELEMETRY_DRIVER_ERROR;
}

void telemetry_on_tx_complete(void)
{
    transmission_active = false;
}
