/* --bench percentile, formatting and report rules (15-CONTEXT D-05, D-06, D-03). */
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "../check.h"

static int same(const char *got, const char *want)
{
    if (strcmp(got, want) != 0) {
        fprintf(stderr, "got:  %s\nwant: %s\n", got, want);
        return 0;
    }
    return 1;
}

static void shuffled(uint64_t *s, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        s[(uint32_t)(((uint64_t)i * 7919u) % n)] = (uint64_t)(i + 1u) * 1000u;
    }
}

int main(void)
{
    static uint64_t s[1000];
    char buf[512];
    const uint32_t ps[4] = {500u, 900u, 990u, 999u};

    for (int i = 0; i < 4; i++) {
        CHECK_EQ_U64(nesturbator_run_bench_rank(1u, ps[i]), 0);
    }
    CHECK_EQ_U64(nesturbator_run_bench_rank(1000u, 999u), 998);
    CHECK_EQ_U64(nesturbator_run_bench_rank(999u, 999u), 998);

    shuffled(s, 1000u);
    nesturbator_run_bench_series_line(buf, sizeof buf, "frame_us", s, 1000u);
    CHECK(same(buf, "bench frame_us p50=500.000 p90=900.000 p99=990.000 p99.9=999.000 "
                    "max=1000.000"));
    shuffled(s, 999u);
    nesturbator_run_bench_series_line(buf, sizeof buf, "frame_us", s, 999u);
    CHECK(same(buf, "bench frame_us p50=500.000 p90=900.000 p99=990.000 p99.9=999.000 "
                    "max=999.000"));
    s[0] = 1234567u;
    nesturbator_run_bench_series_line(buf, sizeof buf, "frame_us", s, 1u);
    CHECK(same(buf, "bench frame_us p50=1234.567 p90=1234.567 p99=1234.567 p99.9=1234.567 "
                    "max=1234.567"));
    nesturbator_run_bench_format_us(buf, sizeof buf, 5u);
    CHECK(same(buf, "0.005"));

    nesturbator_run_bench_total_line(buf, sizeof buf, 600u, 1234567000u);
    CHECK(same(buf, "bench frames=600 time_ms=1234.567 fps=486.00"));
    nesturbator_run_bench_total_line(buf, sizeof buf, 1u, 0u);
    CHECK(same(buf, "bench frames=1 time_ms=0.000 fps=0.00"));

    /* Zero samples: exactly one line, whatever the pointers (D-03). */
    {
        uint64_t a[1] = {0}, b[1] = {0}, c[1] = {0};
        size_t n = nesturbator_run_bench_report(buf, sizeof buf, 0u, NULL, NULL, NULL);
        CHECK(same(buf, "nesturbator-run: bench frames=0\n"));
        CHECK_EQ_U64(n, strlen(buf));
        n = nesturbator_run_bench_report(buf, sizeof buf, 0u, a, b, c);
        CHECK(same(buf, "nesturbator-run: bench frames=0\n"));
        CHECK_EQ_U64(n, strlen(buf));
    }
    {
        uint64_t f[2] = {3000u, 1000u};
        size_t n = nesturbator_run_bench_report(buf, sizeof buf, 2u, f, NULL, NULL);
        CHECK(same(buf,
                   "nesturbator-run: bench frames=2 time_ms=0.004 fps=500000.00\n"
                   "nesturbator-run: bench frame_us p50=1.000 p90=3.000 p99=3.000 p99.9=3.000 "
                   "max=3.000\n"));
        CHECK_EQ_U64(n, strlen(buf));
    }
    {
        uint64_t f[1] = {2000u}, sv[1] = {5000u}, ld[1] = {7000u};
        nesturbator_run_bench_report(buf, sizeof buf, 1u, f, sv, ld);
        CHECK(same(buf,
                   "nesturbator-run: bench frames=1 time_ms=0.002 fps=500000.00\n"
                   "nesturbator-run: bench frame_us p50=2.000 p90=2.000 p99=2.000 p99.9=2.000 "
                   "max=2.000\n"
                   "nesturbator-run: bench save_us p50=5.000 p90=5.000 p99=5.000 p99.9=5.000 "
                   "max=5.000\n"
                   "nesturbator-run: bench load_us p50=7.000 p90=7.000 p99=7.000 p99.9=7.000 "
                   "max=7.000\n"));
    }
    CHECK_DONE();
}
