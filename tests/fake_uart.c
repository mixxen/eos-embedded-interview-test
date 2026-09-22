#include "fake_uart.h"
#include "telemetry.h"
#include "uart_driver.h"

#include <string.h>

static const uint8_t *borrowed_data;
static size_t borrowed_length;
static uint8_t completed_packet[TELEMETRY_PACKET_SIZE];
static size_t completed_length;
static size_t start_calls;
static bool fail_next_start;

void fake_uart_reset(void)
{
    borrowed_data = NULL;
    borrowed_length = 0;
    completed_length = 0;
    start_calls = 0;
    fail_next_start = false;
    memset(completed_packet, 0, sizeof completed_packet);
}

void fake_uart_fail_next_start(void)
{
    fail_next_start = true;
}

bool fake_uart_is_active(void)
{
    return borrowed_data != NULL;
}

size_t fake_uart_start_calls(void)
{
    return start_calls;
}

bool uart_dma_start(const uint8_t *data, size_t length)
{
    ++start_calls;
    if (fail_next_start) {
        fail_next_start = false;
        return false;
    }
    if (borrowed_data != NULL || data == NULL ||
        length != TELEMETRY_PACKET_SIZE) {
        return false;
    }
    /* Deliberately retain the pointer, not a copy. This models borrowing. */
    borrowed_data = data;
    borrowed_length = length;
    return true;
}

bool fake_uart_complete(void)
{
    if (borrowed_data == NULL) {
        return false;
    }
    /* The caller has already returned from telemetry_send(). */
    memcpy(completed_packet, borrowed_data, borrowed_length);
    completed_length = borrowed_length;
    borrowed_data = NULL;
    borrowed_length = 0;
    telemetry_on_tx_complete();
    return true;
}

const uint8_t *fake_uart_last_packet(void)
{
    return completed_packet;
}

size_t fake_uart_last_length(void)
{
    return completed_length;
}
