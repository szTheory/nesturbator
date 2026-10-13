---
phase: 11-tune-up-and-state-readiness
plan: 01
subsystem: runner
tags: [save, atomic-write, tests]
requires: []
provides: [nesturbator_run_file_replace]
affects: [runner/save.c, runner/save.h, runner/main.c, README.md]
key-files:
  created: [tests/runner/test_save_replace.c, tests/runner/test_save_fsize.c]
  modified: [runner/save.c, runner/save.h, runner/main.c, tests/CMakeLists.txt, tests/cmake/runner_save.cmake, README.md]
decisions:
  - "Only the exit flush retries after a failed interval flush (CONTEXT D-04 corrected); docs match the code"
requirements: [TUNE-07]
status: complete
commits: 2
plan_head_before: c270511b560553b710c9ce39e0cfb9573002b448
plan_head_after: 87482e0
actuals:
  tokens: 14000
  tasks: 3
  commits: 2
---

# Phase 11 Plan 01: Atomic save replace Summary

One shared `nesturbator_run_file_replace` helper (stat-guarded `.tmp`, exclusive create, fsync, checked close, rename) now backs the battery flush, with failure-injection tests proving the old save survives every failure.

## Done
- Task 1-2: helper, wrapper, `runner.save_replace` (all platforms plus chmod and symlink on UNIX), `runner.save_fsize` (UNIX), `runner.save.dir_at_tmp`, updated message matches. Committed together in 5e45b8b because the tests share files.
- Task 3: `battery_save` rename, header, main.c comment and README in 87482e0.
- `cmake --workflow --preset ci`: 485/485 passed. No change under src/ or include/.

## Deviations from Plan
- Tasks 1 and 2 landed as one commit (overlapping files). The retry wording follows the code: a failed interval flush ends the run with status 1 and the exit flush retries once.
- The README has no earlier test-description area for runner.save; the test paragraph was added after the `--save-interval` bullet.
- asan workflow and hygiene.sh were not run to completion separately (hygiene printed no errors).

## Known Stubs
None.

## Self-Check: PASSED
