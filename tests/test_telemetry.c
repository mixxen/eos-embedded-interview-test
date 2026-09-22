#include "telemetry.h"
#include "fake_uart.h"
#include "test_helpers.h"

#include <string.h>

static bool packet_encodes_little_endian(void)
{
    uint8_t output[TELEMETRY_PACKET_SIZE] = {0};
    const uint8_t expected[] = {0xA5, 0x07, 0x34, 0x12};
    CHECK(telemetry_encode(7, 0x1234, output, sizeof output));
    CHECK(memcmp(output, expected, sizeof expected) == 0);
    return true;
}

static bool packet_accepts_zero_values(void)
{
    uint8_t output[TELEMETRY_PACKET_SIZE] = {0};
    const uint8_t expected[] = {0xA5, 0x00, 0x00, 0x00};
    CHECK(telemetry_encode(0, 0, output, sizeof output));
    CHECK(memcmp(output, expected, sizeof expected) == 0);
    return true;
}

static bool packet_rejects_null(void)
{
    CHECK(!telemetry_encode(7, 0x1234, NULL, TELEMETRY_PACKET_SIZE));
    CHECK(!telemetry_encode(7, 0x1234, NULL, 0));
    return true;
}

static bool packet_rejects_short_buffer_without_writes(void)
{
    uint8_t output[TELEMETRY_PACKET_SIZE] = {0xCC, 0xCC, 0xCC, 0xCC};
    const uint8_t unchanged[] = {0xCC, 0xCC, 0xCC, 0xCC};
    for (size_t capacity = 0; capacity < TELEMETRY_PACKET_SIZE; ++capacity) {
        CHECK(!telemetry_encode(7, 0x1234, output, capacity));
        CHECK(memcmp(output, unchanged, sizeof unchanged) == 0);
    }
    return true;
}

static bool packet_leaves_extra_capacity_untouched(void)
{
    uint8_t output[] = {0, 0, 0, 0, 0xCC, 0xDD};
    const uint8_t expected[] = {0xA5, 0x07, 0x34, 0x12, 0xCC, 0xDD};
    CHECK(telemetry_encode(7, 0x1234, output, sizeof output));
    CHECK(memcmp(output, expected, sizeof expected) == 0);
    return true;
}

static bool packet_accepts_exact_capacity_without_overrun(void)
{
    uint8_t guarded[] = {0xCC, 0, 0, 0, 0, 0xDD};
    const uint8_t expected[] = {0xCC, 0xA5, 0x02, 0xCD, 0xAB, 0xDD};
    CHECK(telemetry_encode(2, 0xABCD, &guarded[1], TELEMETRY_PACKET_SIZE));
    CHECK(memcmp(guarded, expected, sizeof expected) == 0);
    return true;
}

static bool send_keeps_bytes_valid_until_completion(void)
{
    const uint8_t expected[] = {0xA5, 0x07, 0x34, 0x12};
    CHECK(telemetry_send(7, 0x1234) == TELEMETRY_STARTED);
    CHECK(fake_uart_is_active());
    CHECK(fake_uart_start_calls() == 1);
    CHECK(fake_uart_last_length() == 0); /* Not completed yet. */
    CHECK(fake_uart_complete());
    CHECK(fake_uart_last_length() == sizeof expected);
    CHECK(memcmp(fake_uart_last_packet(), expected, sizeof expected) == 0);
    return true;
}

static bool send_rejects_busy_without_calling_driver(void)
{
    CHECK(telemetry_send(7, 0x1234) == TELEMETRY_STARTED);
    CHECK(telemetry_send(9, 0xBEEF) == TELEMETRY_BUSY);
    CHECK(fake_uart_start_calls() == 1);
    CHECK(fake_uart_is_active());
    CHECK(fake_uart_complete());
    return true;
}

static bool send_busy_attempt_does_not_overwrite_pending_bytes(void)
{
    const uint8_t expected[] = {0xA5, 0x07, 0x34, 0x12};
    CHECK(telemetry_send(7, 0x1234) == TELEMETRY_STARTED);
    CHECK(telemetry_send(9, 0xBEEF) == TELEMETRY_BUSY);
    CHECK(fake_uart_complete());
    CHECK(memcmp(fake_uart_last_packet(), expected, sizeof expected) == 0);
    return true;
}

static bool send_allows_reuse_after_completion(void)
{
    const uint8_t expected[] = {0xA5, 0x09, 0xEF, 0xBE};
    CHECK(telemetry_send(7, 0x1234) == TELEMETRY_STARTED);
    CHECK(fake_uart_complete());
    CHECK(!fake_uart_is_active());
    CHECK(telemetry_send(9, 0xBEEF) == TELEMETRY_STARTED);
    CHECK(fake_uart_complete());
    CHECK(fake_uart_start_calls() == 2);
    CHECK(memcmp(fake_uart_last_packet(), expected, sizeof expected) == 0);
    return true;
}

static bool send_reports_driver_start_failure(void)
{
    fake_uart_fail_next_start();
    CHECK(telemetry_send(7, 0x1234) == TELEMETRY_DRIVER_ERROR);
    CHECK(fake_uart_start_calls() == 1);
    CHECK(!fake_uart_is_active());
    CHECK(fake_uart_last_length() == 0);
    return true;
}

static bool send_allows_retry_after_driver_failure(void)
{
    const uint8_t expected[] = {0xA5, 0x09, 0xEF, 0xBE};
    fake_uart_fail_next_start();
    CHECK(telemetry_send(7, 0x1234) == TELEMETRY_DRIVER_ERROR);
    CHECK(telemetry_send(9, 0xBEEF) == TELEMETRY_STARTED);
    CHECK(fake_uart_complete());
    CHECK(fake_uart_start_calls() == 2);
    CHECK(memcmp(fake_uart_last_packet(), expected, sizeof expected) == 0);
    return true;
}

typedef struct {
    const char *group;
    const char *name;
    bool (*run)(void);
} test_case_t;

int main(int argc, char *argv[])
{
    const test_case_t tests[] = {
        {"packet", "little-endian packet", packet_encodes_little_endian},
        {"packet", "zero values", packet_accepts_zero_values},
        {"packet", "NULL output", packet_rejects_null},
        {"packet", "short buffer / no writes", packet_rejects_short_buffer_without_writes},
        {"packet", "extra capacity untouched", packet_leaves_extra_capacity_untouched},
        {"packet", "exact capacity / guards", packet_accepts_exact_capacity_without_overrun},
        {"send", "delayed completion / lifetime", send_keeps_bytes_valid_until_completion},
        {"send", "busy / no driver call", send_rejects_busy_without_calling_driver},
        {"send", "busy / no overwrite", send_busy_attempt_does_not_overwrite_pending_bytes},
        {"send", "reuse after completion", send_allows_reuse_after_completion},
        {"send", "driver start failure", send_reports_driver_start_failure},
        {"send", "retry after failure", send_allows_retry_after_driver_failure},
        {"candidate", "candidate-added test", candidate_test}
    };
    const char *group = argc == 2 ? argv[1] : "all";
    size_t passed = 0;
    size_t failed = 0;
    if (argc > 2 || (strcmp(group, "all") != 0 &&
                    strcmp(group, "packet") != 0 &&
                    strcmp(group, "send") != 0 &&
                    strcmp(group, "candidate") != 0)) {
        fprintf(stderr, "Usage: %s [all|packet|send|candidate]\n", argv[0]);
        return 2;
    }
    for (size_t index = 0; index < sizeof tests / sizeof tests[0]; ++index) {
        if (strcmp(group, "all") != 0 && strcmp(group, tests[index].group) != 0) {
            continue;
        }
        /* Abandon a previous test's transfer before resetting sender state. */
        fake_uart_reset();
        telemetry_init();
        const bool result = tests[index].run();
        printf("[%s] %s\n", result ? "PASS" : "FAIL", tests[index].name);
        if (result) {
            ++passed;
        } else {
            ++failed;
        }
    }
    printf("\n%zu passed, %zu failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
