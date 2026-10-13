---
phase: 15-speed-and-close-out
plan: 04
subsystem: perf-gate
tags: [cmake, perf, cachegrind, allow-file, ctest]
requires: [15-03]
provides:
  - "tests/cmake/perf_compare.cmake: +1% instruction gate, UNSTABLE stop, Markdown table, allow-file validation"
  - "perf.gate_selftest.* (19 ctests, label perf)"
  - "tests/perf/allow.txt (comments only)"
affects: [15-05]
key-files:
  created: [tests/cmake/perf_compare.cmake, tests/cmake/perf_gate_selftest.cmake, tests/perf/allow.txt, tests/perf/fixtures/cachegrind.out.in]
  modified: [tests/CMakeLists.txt, README.md]
decisions:
  - "A base with either measurement missing prints n/a in both modes; a missing head fails only in gate mode; UNSTABLE fails in every mode and no allow line excuses it."
  - "Allow-file reason text is validated as non-empty and never printed; the summary shows ROM, cap, expiry and line number only."
  - "The self-test writes each case's allow text itself, so tests/perf/fixtures holds one file."
metrics:
  duration: 20m
  completed: 2026-10-12
status: complete
commits: 3
plan_head_before: 8ba71df
plan_head_after: d4215d8ba85e90dbebc6f44bbdcdcaf56c62fcd8
actuals:
  tokens: 12000
  tasks: 2
  commits: 3
---

# Phase 15 Plan 04: Performance comparison script and allow file Summary

One CMake script compares Cachegrind instruction counts (d = Ir(360) - Ir(60), fails when (d_head - d_base) * 100 > d_base), prints the D-12 table, validates `tests/perf/allow.txt` (seven failure messages, 60-day cap), and 19 `perf.gate_selftest.*` ctests prove each rule on synthetic files.

## Commits

- ef11889 feat(15-04): comparison script, fixture self-test (8 cases)
- d80ba0c test(15-04): allow-file cases (RED, 10 failing)
- d4215d8 feat(15-04): allow-file validation, ALLOWED status, README paragraph (GREEN)

## Sample summary.md, `identical` case

```
| ROM | mapper | base Ir/frame | head Ir/frame | delta Ir/frame | delta % | status |
|---|---|---|---|---|---|---|
| nesteroids | 0 | 10000 | 10000 | +0 | +0.00% | ok |
| runner | 1 | 10000 | 10000 | +0 | +0.00% | ok |
| M4_P256K_C256K | 4 | 10000 | 10000 | +0 | +0.00% | ok |

base n/a head n/a
valgrind: n/a
gcc: n/a
Ir/frame = (Ir(360) - Ir(60)) / 300; FAIL when (d_head - d_base) * 100 > d_base
```

## Sample summary.md, `allow_ok` case (+1.01% with a reviewed line)

```
| nesteroids | 0 | 10000 | 10101 | +101 | +1.01% | ALLOWED |
...
ALLOWED nesteroids cap 1.50% expires 2026-11-30 (tests/perf/allow.txt line 2)
```

The reason word from the allow line does not appear (the case forbids it).

## Deviations from Plan

None in behavior. The self-test echoes the comparison output unwrapped (execute_process echo) so the ALLOWED line is whole in `ctest -V`; `message()` wraps long lines. The `improvement` case expects -3333 per frame (2000000 - 3000000 over 300), corrected from a first miscalculation of the expectation.

## Verification

- `cmake --workflow --preset ci` passes; `cmake --workflow --preset hygiene` passes (8 of 8).
- 19 `perf.gate_selftest.*` tests pass; `ls tests/perf/fixtures` shows one file.

## Known Stubs

None.

## Self-Check: PASSED

Files and commits ef11889, d80ba0c, d4215d8 exist on the branch.
