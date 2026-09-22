#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Supplied by tests/fake_uart.c; do not implement or change this function.
 * true: borrows data (does NOT copy it) until telemetry_on_tx_complete().
 * false: borrows nothing; no callback will follow for this attempt.
 * Returns immediately. Completion never runs inside this call or before
 * telemetry_send() returns. No other code uses this UART in the exercise.
 * Completion means the buffer can be reused, not a guarantee about every
 * real UART's wire-level completion interrupt.
 */
bool uart_dma_start(const uint8_t *data, size_t length);

#endif
