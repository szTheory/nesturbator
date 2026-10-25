/* Dormant, test-only hooks for exact failure/minimization controls. */
#ifndef NESTURBATOR_FUZZ_FAILURE_CONTROL_H
#define NESTURBATOR_FUZZ_FAILURE_CONTROL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void nt_fuzz_trace_operation(const char *name)
{
    if (getenv("NESTURBATOR_FUZZ_TRACE_OPERATION") != NULL) {
        fprintf(stderr, "fuzz.operation=%s\n", name);
        (void)fflush(stderr);
    }
}

static int nt_fuzz_control_matches(const char *target, const uint8_t *data, size_t size,
                                   size_t marker_offset, uint8_t marker)
{
    const char *control = getenv("NESTURBATOR_FUZZ_CONTROL");
    return control != NULL && strcmp(control, target) == 0 && data != NULL &&
           marker_offset < size && data[marker_offset] == marker;
}

static void nt_fuzz_control_crash(const char *target)
{
    volatile uint8_t *bytes = (volatile uint8_t *)malloc(1u);
    volatile size_t one_past_end = 1u;
    if (bytes == NULL)
        abort();
    fprintf(stderr, "fuzz.control=synthetic-sanitizer-fixture target=%s\n", target);
    (void)fflush(stderr);
    bytes[one_past_end] = 0xa5u;
}

#endif
