/* Frame-time statistics for --bench (15-CONTEXT D-05, D-06).

   Runner-only and integer-only. Every value is a measured sample: percentiles use the nearest-rank
   method (rank = ceil(p * n), no interpolation), so p99.9 equals the maximum below 1000 samples.
   Times print in microseconds with three decimals, truncated from nanoseconds. */
#ifndef NESTURBATOR_RUNNER_BENCH_H
#define NESTURBATOR_RUNNER_BENCH_H

#include <stddef.h>
#include <stdint.h>

/* Zero-based index of the nearest-rank percentile in n >= 1 sorted samples. p_x10 is the
   percentile times ten: 500, 900, 990, 999. */
uint32_t nesturbator_run_bench_rank(uint32_t n, uint32_t p_x10);

/* Sorts n samples ascending. */
void nesturbator_run_bench_sort(uint64_t *s, uint32_t n);

/* Writes ns as microseconds, "1234.567", truncating. */
void nesturbator_run_bench_format_us(char *buf, size_t cap, uint64_t ns);

/* Sorts s in place, writes "bench <key> p50=.. p90=.. p99=.. p99.9=.. max=.." (n >= 1). */
void nesturbator_run_bench_series_line(char *buf, size_t cap, const char *key, uint64_t *s,
                                       uint32_t n);

/* Writes "bench frames=N time_ms=X.XXX fps=Y.YY"; fps is 0.00 when total_ns is 0. */
void nesturbator_run_bench_total_line(char *buf, size_t cap, uint32_t frames, uint64_t total_ns);

/* Writes the whole report into buf, every line prefixed "nesturbator-run: " and ending in a
   newline, and returns its length. count 0 writes only "nesturbator-run: bench frames=0". The
   save and load series lines follow the frame line only when both arrays are non-NULL. The arrays
   are sorted in place. */
size_t nesturbator_run_bench_report(char *buf, size_t cap, uint32_t count, uint64_t *frame_ns,
                                    uint64_t *save_ns, uint64_t *load_ns);

#endif /* NESTURBATOR_RUNNER_BENCH_H */
