/* Dormant, test-only hooks for exact failure/minimization controls. */
#ifndef NESTURBATOR_FUZZ_FAILURE_CONTROL_H
#define NESTURBATOR_FUZZ_FAILURE_CONTROL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int nt_fuzz_environment_present(const char *name)
{
#if defined(_MSC_VER)
    char *value = NULL;
    size_t length = 0u;
    const int present = _dupenv_s(&value, &length, name) == 0 && value != NULL;
    free(value);
    return present;
#else
    return getenv(name) != NULL;
#endif
}

static int nt_fuzz_environment_matches(const char *name, const char *expected)
{
#if defined(_MSC_VER)
    char *value = NULL;
    size_t length = 0u;
    const int found = _dupenv_s(&value, &length, name) == 0 && value != NULL;
    const int matches = found && strcmp(value, expected) == 0;
    free(value);
    return matches;
#else
    const char *value = getenv(name);
    return value != NULL && strcmp(value, expected) == 0;
#endif
}

static void nt_fuzz_trace_operation(const char *name)
{
    if (nt_fuzz_environment_present("NESTURBATOR_FUZZ_TRACE_OPERATION")) {
        fprintf(stderr, "fuzz.operation=%s\n", name);
        (void)fflush(stderr);
    }
}

static int nt_fuzz_control_matches(const char *target, const uint8_t *data, size_t size,
                                   size_t marker_offset, uint8_t marker)
{
    return nt_fuzz_environment_matches("NESTURBATOR_FUZZ_CONTROL", target) && data != NULL &&
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
