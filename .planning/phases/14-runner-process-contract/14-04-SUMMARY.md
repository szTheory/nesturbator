---
phase: 14-runner-process-contract
plan: 04
subsystem: runner
tags: [signal, raise-after, ctest, exit-status]

requires:
  - phase: 14-02
    provides: stop module, --raise-after, runner_stop_case
  - phase: 14-03
    provides: README stop contract
provides:
  - runner.stop.{flush_missing_dir,flush_dir_at_tmp,at_zero,nodir,double,double.int,load_state_at,load_state_below}
  - runner.usage.raise_after.{bad_sig,foreign_sig,bad_k,twice,beyond,malformed,accuracycoin}
affects: [14-05, 14-06, 14-07]

actuals:
  tokens: 4000
  tasks: 2
  commits: 1

plan_head_before: 71a8308
plan_head_after: a9d133f1bead134e9ff4da6f9c0705ee6d2eeca7

tech-stack:
  added: []
  patterns: ["platform-differing cases branch on CMAKE_HOST_WIN32 / WIN32 inside the script or at registration, never by skip"]

key-files:
  created: []
  modified: [tests/cmake/runner_stop.cmake, tests/CMakeLists.txt]

key-decisions:
  - "Both tasks edit the same two files and landed as one commit"

requirements-completed: [RUN-02, RUN-03]

coverage:
  - id: D1
    description: "A failed exit flush after a stop exits 1 with cannot-save and the stop line; empty and directory-less stops exit 0 and touch nothing"
    requirement: RUN-02
    verification:
      - kind: unit
        ref: "ctest runner.stop.flush_missing_dir, flush_dir_at_tmp, at_zero, nodir"
        status: pass
    human_judgment: false
  - id: D2
    description: "A second signal kills the runner at once, prints no stop line and leaves the seed save whole"
    requirement: RUN-02
    verification:
      - kind: unit
        ref: "ctest runner.stop.double, runner.stop.double.int"
        status: pass
    human_judgment: false
  - id: D3
    description: "--raise-after at a loaded state's frame stops before the first frame; below it or beyond --frames, and every malformed use, is exit 2 naming --raise-after"
    requirement: RUN-03
    verification:
      - kind: unit
        ref: "ctest runner.stop.load_state_at, load_state_below, runner.usage.raise_after.*"
        status: pass
    human_judgment: false
  - id: D4
    description: "Windows registrations (double with break, foreign_sig with term) are compiled out on this host; Plan 07 CI runs them"
    requirement: RUN-02
    verification: []
    human_judgment: false

duration: 6min
completed: 2026-10-12
status: complete
---

# Phase 14 Plan 04: Remaining in-process stop cases Summary

**Eight more `runner.stop.*` cases and seven `runner.usage.raise_after.*` tests prove the failed-flush exit 1, empty and directory-less stops, the second-signal kill that leaves the seed save untouched, load-state adjacency, and every --raise-after misuse as exit 2.**

## Accomplishments
- Flush failures use the existing missing-directory and directory-at-`.sav.tmp` shapes (no chmod); both exit 1 with `cannot save` and the stop line, and the foreign directory is left alone.
- `at_zero` and `nodir` exit 0 with the stop line `after frame 0` / `frame 2`, seed byte-equal, nothing else written.
- `double` and `double.int` require death by signal (POSIX) or an exit other than 0/1 (Windows), no stop line, and an 8192-byte battery.sav equal to the seed.
- `load_state_at` stops at the loaded state's frame; `load_state_below` is exit 2.

## Task Commits
1. Tasks 1 and 2 (same two files): `a9d133f` (test)

## Deviations from Plan
None - plan executed exactly as written. The two tasks share files, so they are one commit.

## Verification
- `cmake --workflow --preset ci`: 569/569 passed; asan workflow and hygiene workflow passed.
- No `chmod`, `SKIP_RETURN_CODE` or `DISABLED` in runner_stop.cmake.

## Next Phase Readiness
Ready for 14-05 (unmet-output cases). Windows branches are read in Plan 07 CI.

## Self-Check: PASSED
