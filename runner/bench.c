#include "bench.h"

#include <stdio.h>
#include <stdlib.h>

#define REPORT_PREFIX "nesturbator-run: "

/* Nearest rank: the smallest sample with at least p of the samples at or below it, so the
   zero-based index is ceil(p * n) - 1 (NIST e-Handbook of Statistical Methods, percentile; no
   interpolation). p_x10 is the percentile times ten. */
uint32_t nesturbator_run_bench_rank(uint32_t n, uint32_t p_x10)
{
    return (uint32_t)(((uint64_t)p_x10 * n + 999u) / 1000u - 1u);
}

static int compare_u64(const void *a, const void *b)
{
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}

void nesturbator_run_bench_sort(uint64_t *s, uint32_t n)
{
    qsort(s, n, sizeof s[0], compare_u64);
}

void nesturbator_run_bench_format_us(char *buf, size_t cap, uint64_t ns)
{
    (void)snprintf(buf, cap, "%llu.%03llu", (unsigned long long)(ns / 1000u),
                   (unsigned long long)(ns % 1000u));
}

void nesturbator_run_bench_series_line(char *buf, size_t cap, const char *key, uint64_t *s,
                                       uint32_t n)
{
    char v[4][32];
    char mx[32];
    static const uint32_t ps[4] = {500u, 900u, 990u, 999u};
    nesturbator_run_bench_sort(s, n);
    for (int i = 0; i < 4; i++) {
        nesturbator_run_bench_format_us(v[i], sizeof v[i], s[nesturbator_run_bench_rank(n, ps[i])]);
    }
    nesturbator_run_bench_format_us(mx, sizeof mx, s[n - 1u]);
    (void)snprintf(buf, cap, "bench %s p50=%s p90=%s p99=%s p99.9=%s max=%s", key, v[0], v[1], v[2],
                   v[3], mx);
}

void nesturbator_run_bench_total_line(char *buf, size_t cap, uint32_t frames, uint64_t total_ns)
{
    /* fps in hundredths, rounded to nearest: frames * 1e9 / total_ns * 100. */
    uint64_t x = total_ns == 0u ? 0u
                                : ((uint64_t)frames * 100u * 1000000000u + total_ns / 2u) / total_ns;
    (void)snprintf(buf, cap, "bench frames=%lu time_ms=%llu.%03llu fps=%llu.%02llu",
                   (unsigned long)frames, (unsigned long long)(total_ns / 1000000u),
                   (unsigned long long)((total_ns / 1000u) % 1000u), (unsigned long long)(x / 100u),
                   (unsigned long long)(x % 100u));
}

static size_t append_line(char *buf, size_t cap, size_t len, const char *line)
{
    int w;
    if (len >= cap) {
        return len;
    }
    w = snprintf(buf + len, cap - len, REPORT_PREFIX "%s\n", line);
    if (w < 0) {
        return len;
    }
    len += (size_t)w;
    return len < cap ? len : cap - 1u;
}

size_t nesturbator_run_bench_report(char *buf, size_t cap, uint32_t count, uint64_t *frame_ns,
                                    uint64_t *save_ns, uint64_t *load_ns)
{
    char line[200];
    size_t len = 0u;
    uint64_t total = 0u;
    if (cap == 0u) {
        return 0u;
    }
    buf[0] = '\0';
    if (count == 0u) {
        return append_line(buf, cap, len, "bench frames=0");
    }
    for (uint32_t i = 0; i < count; i++) {
        total += frame_ns[i];
    }
    nesturbator_run_bench_total_line(line, sizeof line, count, total);
    len = append_line(buf, cap, len, line);
    nesturbator_run_bench_series_line(line, sizeof line, "frame_us", frame_ns, count);
    len = append_line(buf, cap, len, line);
    if (save_ns != NULL && load_ns != NULL) {
        nesturbator_run_bench_series_line(line, sizeof line, "save_us", save_ns, count);
        len = append_line(buf, cap, len, line);
        nesturbator_run_bench_series_line(line, sizeof line, "load_us", load_ns, count);
        len = append_line(buf, cap, len, line);
    }
    return len;
}
