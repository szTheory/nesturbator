---
phase: 15-speed-and-close-out
plan: 02
subsystem: runner
tags: [runner, bench, performance, ctest]
requires: [15-01]
provides:
  - "nesturbator-run --bench N (frame-time percentiles on stderr)"
  - "runner/clock.[ch], runner/bench.[ch]"
  - "runner.bench.percentile, runner.bench.nohash, runner.bench.format"
affects: [15-03]
key-files:
  created: [runner/clock.c, runner/clock.h, runner/bench.c, runner/bench.h, tests/runner/test_bench.c, tests/cmake/runner_bench.cmake]
  modified: [runner/main.c, runner/CMakeLists.txt, tests/CMakeLists.txt, README.md]
decisions:
  - "The bench report is one helper; main passes the timed count ungated so the helper alone decides the zero-sample output."
metrics:
  duration: 25m
  completed: 2026-10-12
status: complete
commits: 3
plan_head_before: 35e17aa9e8e364e0ac985d6346b77cb1c362086b
plan_head_after: 78646f5fddd7b385331c179920217990f7c074c8
actuals:
  tokens: 14000
  tasks: 2
  commits: 3
---

# Phase 15 Plan 02: --bench N Summary

`nesturbator-run --rom R --bench N` times each frame with a monotonic clock around `nesturbator_run_frame` only and prints total, fps and nearest-rank p50/p90/p99/p99.9/max on stderr, with stdout and hashes identical to `--frames N`.

## What was built

- `runner/clock.c`: `nesturbator_run_clock_now_ns` (CLOCK_MONOTONIC, or QueryPerformanceCounter with a sec/remainder split on Windows); opens with `_POSIX_C_SOURCE 199309L`.
- `runner/bench.c`: rank `(p_x10*N+999)/1000-1`, qsort, integer microsecond formatting, series and total lines, and `nesturbator_run_bench_report` (count 0 writes only `nesturbator-run: bench frames=0`). No floating point.
- `runner/main.c`: `--bench` parsing (1 to 1000000, once, not with `--frames`/`--movie`, needs `--rom`), sample array in the options, timed frame call, report before the stop flush and stop line; `--help` text.
- README `--bench N` bullet and `--frames` required sentence.

## Tests added

- `runner.bench.percentile` (unit: ranks, 1000 and 999 shuffled samples, single sample, total line `fps=486.00`, zero-sample output, two- and four-line reports), ci and asan.
- `runner.bench.nohash` (`--frames K` vs `--bench K` stdout byte-identical, exit 0).
- `runner.bench.format` (line shapes and keys only, no numbers; STATE=1 variant ready for Plan 03).

## Sample `--bench 600` stderr (this Mac, arm64)

```
nesturbator-run: bench frames=600 time_ms=701.367 fps=855.47
nesturbator-run: bench frame_us p50=1163.000 p90=1235.000 p99=1339.000 p99.9=1616.000 max=1616.000
```

## Zero-sample and stop-order checks (W2, I2)

- Source shape: lines around `nesturbator_run_bench_report(` matching a `bench_timed` test: 0; the count is passed on the call line.
- Scratch-copy mutation: with `if (bench_timed > 0)` inserted above the call the grep reported 1 (check fails); after restoring from the scratch copy `cmp` was identical and the grep reported 0.
- Order in main.c: bench report (line 1385) < "stopped by signal after frame" (1399) < `report_unmet(` call (1401).
- Usage exits: `--bench 0`, `--bench 1000001`, `--bench 5 --frames 5`, `--bench 5` without `--rom` all exit 2; `--frames 1` exits 0.

## Commits

- d1b83eb test(15-02): bench percentile, format and zero-sample report rules (RED)
- bf416ea feat(15-02): nearest-rank bench statistics and report helper (GREEN)
- 78646f5 feat(15-02): nesturbator-run --bench N with frame-time percentiles

## Deviations from Plan

None in behaviour. clang-format 18 lives at `/opt/homebrew/opt/llvm@18/bin/clang-format` (not on PATH); hygiene failed on first run, so the new and edited C files were formatted with it and hygiene then passed (this also reflowed `print_usage`).

## Verification

`cmake --workflow --preset ci` (590 tests), `asan` (585 tests) and `hygiene` pass locally. `nofp` not run (known macOS stack-protector failure).

## Known Stubs

None.

## Self-Check: PASSED
