/* The runner's monotonic clock for --bench (15-CONTEXT D-04).

   Runner-only: the core stays integer-only and clock-free. */
#ifndef NESTURBATOR_RUNNER_CLOCK_H
#define NESTURBATOR_RUNNER_CLOCK_H

#include <stdint.h>

/* Returns a monotonic reading in nanoseconds, or 0 when the clock cannot be read. Only the
   difference between two readings means anything. */
uint64_t nesturbator_run_clock_now_ns(void);

#endif /* NESTURBATOR_RUNNER_CLOCK_H */
