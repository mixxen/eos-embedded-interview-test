#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdbool.h>
#include <stdio.h>

/* Unlike assert(), CHECK remains active in Release builds. */
#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            fprintf(stderr, "%s:%d: check failed: %s\n",                      \
                    __FILE__, __LINE__, #condition);                          \
            return false;                                                    \
        }                                                                    \
    } while (0)

bool candidate_test(void);

#endif
