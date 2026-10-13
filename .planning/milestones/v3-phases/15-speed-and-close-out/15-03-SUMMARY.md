---
phase: 15-speed-and-close-out
plan: 03
subsystem: runner
tags: [runner, bench, signal, ctest]
requires: [15-02]
provides:
  - "nesturbator-run --bench-state (timed save and load per frame)"
  - "D-03 refusals, runner.usage.bench.* (16 tests)"
  - "runner.signal.bench, runner.signal.bench_int"
affects: [15-04]
key-files:
  modified: [runner/main.c, tests/CMakeLists.txt, tests/cmake/runner_bench.cmake, tests/runner/test_signal.c, README.md]
decisions:
  - "Readiness for the bench signal test is the flushed frame 1 hash line on the stdout pipe, flushed only under --bench and outside the timed region."
metrics:
  duration: 25m
  completed: 2026-10-12
status: complete
commits: 3
plan_head_before: dde850c100d34caada32b11d294ab8fa09cb571d
plan_head_after: 7d80d8bbc26ab0affbc936debb8f560de0d52956
actuals:
  tokens: 9000
  tasks: 3
  commits: 3
---

# Phase 15 Plan 03: --bench-state, refusals and signal stop Summary

`--bench-state` saves and loads the state after every timed frame, times each separately and adds `save_us` and `load_us` lines; `--bench` refuses eight file-I/O options with exit 2; a real SIGTERM or SIGINT during `--bench` prints the stats, then the stop line, exit 0.

## Sample `--bench 600 --bench-state` stderr (this Mac, arm64)

```
nesturbator-run: bench frames=600 time_ms=696.379 fps=861.60
nesturbator-run: bench frame_us p50=1152.000 p90=1217.000 p99=1399.000 p99.9=1477.000 max=1477.000
nesturbator-run: bench save_us p50=9.000 p90=10.000 p99=13.000 p99.9=15.000 max=15.000
nesturbator-run: bench load_us p50=10.000 p90=11.000 p99=13.000 p99.9=27.000 max=27.000
```

## Tests added

- `runner.bench.format_state`; `runner.bench.nohash` now has a third run (`--bench K --bench-state`) compared byte for byte.
- `runner.usage.bench.*`: `accuracycoin-page`, `raise-after`, `dump-frame`, `state-at-frame`, `hash-state`, `save-dir`, `save-interval`, `load-state`, `frames`, `movie`, `zero`, `over_cap`, `twice`, `no_rom`, `state_without_bench`, `state_twice` (16, label exit_status).
- `runner.signal.bench` (SIGTERM) and `runner.signal.bench_int` (SIGINT): both passed; wait for `frame 1 ` on the pipe, signal, exit 0, `bench frames=` and `bench frame_us` before the stop line.

## Why the save/load failure branch has no ci test (W3)

No command line reaches it without a production-only fault-injection hook. Save fails only for a NULL instance or buffer, no cartridge, or a too-small capacity; under `--bench` the cartridge is loaded, the buffer is sized from `nesturbator_state_size` before the loop, and the load reads the bytes the same instance just saved. The `--save-dir` failure tests fail file I/O, which `--bench-state` never does. The branch is defensive; both messages and `RUNNER_EXIT_FAILURE` are pinned in source.

## Commits

- 505c13b feat(15-03): --bench-state times a state save and load after every frame
- c84b228 feat(15-03): --bench refuses options that would pollute the timing
- 7d80d8b feat(15-03): flush --bench hash lines and test a real signal stop

## Deviations from Plan

None. The tests for Task 3 passed first time after the flush, so it is one feat commit (the plan allows this). clang-format 18 was applied to main.c and test_signal.c.

## Verification

`cmake --workflow --preset ci`, `asan` and `hygiene` pass locally. `nofp` not run (known macOS failure).

## Known Stubs

None.

## Self-Check: PASSED
