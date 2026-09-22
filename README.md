# EO Solutions — Embedded Software Coding Exercise

**30 minutes · C11 · No development board or RTOS required**

Complete a small telemetry sender. A sensor reading becomes a four-byte packet,
then a simulated UART driver transmits it asynchronously using a DMA-style
interface. The driver returns before it has finished reading the packet.

We are interested in your reasoning, memory handling, testing, and communication,
not typing speed or familiarity with a particular microcontroller SDK. Partial
solutions are useful: explain what works and what you would investigate next.

## Before the timer starts

Clone this repository, or download it using **Code > Download ZIP**. You need a C
compiler and CMake 3.16 or later. No third-party C libraries or downloads are needed once
the tools are installed. Have your interviewer resolve setup issues before the
30-minute exercise starts.

To clone the exercise:

```sh
git clone https://github.com/mixxen/eos-embedded-interview-test.git
cd eos-embedded-interview-test
```

From the project directory (or the extracted ZIP directory):

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug -L candidate --output-on-failure
```

The starter **compiles but intentionally fails tests**: 2 checks pass and 11
fail across 13 test cases. CTest organizes these into three groups. The
candidate-added test also fails until you implement it. A completed solution
should pass all 13 cases.

No CMake available? With GCC or Clang on Linux/macOS/WSL:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Iinclude -Itests src/telemetry.c tests/fake_uart.c tests/test_telemetry.c tests/test_candidate.c -o telemetry_tests
./telemetry_tests
```

## Your tasks

Edit only **`src/telemetry.c`** and **`tests/test_candidate.c`**. Read
`include/telemetry.h` and `include/uart_driver.h` for the full contracts. The
initialization function, completion callback, fake driver, and test runner are
already implemented. Do not weaken the supplied tests or change the driver API.

### 1. Encode a packet — about 8 minutes

Complete `telemetry_encode()`:

| Byte | Meaning | Example |
|---|---|---|
| 0 | Fixed start marker | `0xA5` |
| 1 | Sensor identifier | `0x07` |
| 2 | Low byte of the reading | `0x34` |
| 3 | High byte of the reading | `0x12` |

For sensor `7` and reading `0x1234`, output is **`A5 07 34 12`**.
This ordering is called little-endian. Encode the bytes explicitly; do not send
a C struct's raw memory.

Return `false` for a null output pointer or capacity below four, **without
writing anything**. Otherwise write exactly four bytes and return `true`.
All possible `uint8_t` identifiers and `uint16_t` readings are valid. Extra
output capacity must remain untouched. This simplified interview protocol has
no checksum or packet parser to implement.

### 2. Send without corrupting an active transfer — about 11 minutes

Complete `telemetry_send()` using your encoder and `uart_dma_start()`.

The driver **borrows your buffer; it does not copy it**. On an accepted start,
the bytes must remain valid and unchanged until `telemetry_on_tx_complete()`.
In tests, `fake_uart_complete()` reads those bytes and invokes that callback.

| Situation | Required result |
|---|---|
| Driver accepts a new packet | Return `TELEMETRY_STARTED`. |
| A previous packet is still in progress | Return `TELEMETRY_BUSY`; do not change its bytes or call the driver. |
| Driver refuses to start | Return `TELEMETRY_DRIVER_ERROR`; a later call must be able to retry. |
| Completion callback has run | A new send is allowed. |

Use the existing `transmission_active` state and choose appropriate buffer
storage. Do not use heap allocation, waiting loops, queues, or delays.

**Scope:** One sender and one UART. Calls and callbacks are serialized; there
are no actual threads or interrupts in this test. Completion cannot occur
inside `uart_dma_start()` or before `telemetry_send()` returns. There is no
callback after a failed start. You do not need to solve RTOS synchronization.

### 3. Add one test — about 5 minutes

Implement `candidate_test()` in `tests/test_candidate.c`. Add at least one
meaningful assertion using `CHECK(...)`, then return `true` on success. The test
runner resets the sender and fake driver before each case.

Choose an edge case or failure sequence and explain which plausible bug your
test would catch. Read `tests/test_telemetry.c` for examples. Simply replacing
the placeholder with `return true` is not a test.

## Suggested pace

| Time | Activity |
|---|---|
| 0–3 minutes | Read the contracts and ask questions. |
| 3–11 minutes | Implement and test the packet encoder. |
| 11–22 minutes | Implement and test the sender. |
| 22–27 minutes | Add your test and rerun. |
| 27–30 minutes | Explain your approach and remaining risks. |

Rerun the build after edits. To focus on one part, add `-R candidate.packet`,
`-R candidate.send`, or `-R candidate.candidate` to the CTest command. The
standalone executable also accepts `packet`, `send`, or `candidate`.

Be ready to explain why your storage is safe, what these computer-based tests
prove, and what you would still check on real hardware. Documentation is
welcome. Confirm the AI-tool policy with your interviewer before starting;
AI use is not required, and you should explain and verify any generated code.
