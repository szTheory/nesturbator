#ifndef NESTURBATOR_TEST_PROGRESSION_H
#define NESTURBATOR_TEST_PROGRESSION_H

#include <stdint.h>

typedef struct {
    uint64_t anchor_frame;
    uint64_t observed_frame;
    unsigned boundary_count;
} progression_session;

/* RED-phase contract stub: the tracer test requires this prefix to be judged. */
static inline const char *progression_verdict(const progression_session *session, int prefix)
{
    (void)session;
    (void)prefix;
    return "INCOMPLETE";
}

#endif
