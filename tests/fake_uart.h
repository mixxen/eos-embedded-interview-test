#ifndef FAKE_UART_H
#define FAKE_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Test helpers. The test runner resets the fake and sender before each test. */
void fake_uart_reset(void);
void fake_uart_fail_next_start(void);
bool fake_uart_is_active(void);
size_t fake_uart_start_calls(void);

/* Read the borrowed bytes NOW, then invoke telemetry_on_tx_complete().
 * Return false if there is no active transfer. No background thread exists.
 */
bool fake_uart_complete(void);

/* Last completed transfer, not the currently pending transfer.
 * The returned pointer remains valid; contents change on completion/reset.
 */
const uint8_t *fake_uart_last_packet(void);
size_t fake_uart_last_length(void);

#endif
