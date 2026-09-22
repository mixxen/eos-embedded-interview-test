#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { TELEMETRY_PACKET_SIZE = 4 };
#define TELEMETRY_SYNC_BYTE UINT8_C(0xA5)

typedef enum {
    TELEMETRY_STARTED,
    TELEMETRY_BUSY,
    TELEMETRY_DRIVER_ERROR
} telemetry_result_t;

/* Supplied. Call once before use, with no transfer in progress. */
void telemetry_init(void);

/* TASK 1: Encode [0xA5, sensor_id, reading low byte, reading high byte].
 * Return false for NULL output or capacity < TELEMETRY_PACKET_SIZE.
 * On failure, do not write anything. On success, write exactly four bytes
 * and return true. Every uint8_t sensor_id and uint16_t reading is valid.
 */
bool telemetry_encode(uint8_t sensor_id, uint16_t reading,
                      uint8_t *output, size_t output_capacity);

/* TASK 2: Start a nonblocking transfer through uart_dma_start().
 * While a transfer is active, return TELEMETRY_BUSY without touching the
 * in-flight bytes or calling the driver. Otherwise, encode a new packet
 * into storage that remains valid and unchanged until completion.
 * Return TELEMETRY_STARTED if the driver accepts it, or
 * TELEMETRY_DRIVER_ERROR if it refuses. A refused start must be retryable.
 * No heap allocation, waiting loops, queues, or multiple channels.
 * All calls and completion callbacks are serialized in this exercise.
 */
telemetry_result_t telemetry_send(uint8_t sensor_id, uint16_t reading);

/* Supplied. The driver invokes this only when it no longer needs the buffer. */
void telemetry_on_tx_complete(void);

#endif
