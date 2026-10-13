---
phase: 14-runner-process-contract
plan: 02
subsystem: runner
tags: [signal, graceful-stop, console-ctrl-handler, raise-after, ctest]

requires:
  - phase: 14-01
    provides: RUNNER_EXIT_* status table used by the stop path
provides:
  - runner/stop.{h,c}: POSIX signal() flag handler and Windows SetConsoleCtrlHandler handler
  - runner/stop_internal.h Windows test seam
  - main loop stop check, exit sequence with stop line on stderr, exit 0
  - --raise-after N[:SIG[:K]] test aid
  - runner.stop.frame_exact.* tests
affects: [14-03, 14-04, 14-05, 14-06, 14-07]

actuals:
  tokens: 16000
  tasks: 1
  commits: 1

plan_head_before: 30431b8fc55cc59bb56028a8079568d2e640b7ed
plan_head_after: 320b8c00a01f3164d5b7a27419f02c4eccbfca38

tech-stack:
  added: []
  patterns: ["handler stores a flag only; loop head polls it", "--raise-after signals own process for deterministic tests"]

key-files:
  created: [runner/stop.h, runner/stop.c, runner/stop_internal.h, tests/cmake/runner_stop.cmake]
  modified: [runner/main.c, runner/CMakeLists.txt, tests/CMakeLists.txt]

key-decisions:
  - "Handlers install after check_frame_limits and not for --accuracycoin-page"
  - "A flag first seen after the last frame never reaches a loop-head check, so that run completes normally"

requirements-completed: [RUN-02, RUN-03]

coverage:
  - id: D1
    description: "A signal finishes the current frame, flushes, prints the stop line and exits 0 with output and battery.sav equal to an unstopped run"
    requirement: RUN-02
    verification:
      - kind: unit
        ref: "ctest runner.stop.frame_exact.term, runner.stop.frame_exact.int"
        status: pass
    human_judgment: false
  - id: D2
    description: "Windows half of stop.c and the frame_exact.break/int registrations compile-gated; run by Plan 07 CI"
    requirement: RUN-02
    verification: []
    human_judgment: false

duration: 15min
completed: 2026-10-12
status: complete
---

# Phase 14 Plan 02: Stop module and frame-exact graceful stop Summary

**SIGINT/SIGTERM (POSIX) and Ctrl+C/Ctrl+Break/close (Windows) set a flag that the runner polls before each frame, so a stop finishes the frame, flushes and exits 0, proven by `--raise-after` against an unstopped run.**

## Performance

- **Duration:** about 15 min
- **Completed:** 2026-10-12
- **Tasks:** 1
- **Files modified:** 7 (4 created)

## Accomplishments
- `runner/stop.c` with `_DEFAULT_SOURCE` first, POSIX `signal()` handler that resets to `SIG_DFL` and stores a flag; Windows handler with Interlocked state and a bounded close-event wait.
- `main.c` loop-head check, `done_frame` tracking, exit sequence (stdout flush, save flush, stop line, `stop_done`), `--raise-after` parsing and range checks, help line.
- `runner.stop.frame_exact.{term,int}` on POSIX (`break`,`int` registered on Windows); `--raise-after 1:term:2` exits 143 (second signal takes the default action).

## Task Commits

1. **Task 1: Stop module, loop integration and --raise-after** - `320b8c0` (feat)

## Decisions Made
None - followed plan as specified.

## Deviations from Plan

**1. [Rule 3 - Blocking] Test needed its save directories to exist**
- **Found during:** Task 1 verification
- **Issue:** `--save-dir` does not create directories; the first run exited 1.
- **Fix:** `runner_stop.cmake` creates `a` and `b` under WORK first.
- **Committed in:** `320b8c0`

**Total deviations:** 1. **Impact:** none on behavior.

## Issues Encountered
None beyond the above. The Windows half is not compiled on this host; Plan 07's CI reads it.

## Verification
- `cmake --workflow --preset ci`: 554/554 passed. `cmake --workflow --preset hygiene`: passed.
- GCC 14 strict C17 compile of stop.c and main.c: exit 0. clang-format 18 dry run: exit 0.
- Acceptance greps: first line of stop.c is `#define _DEFAULT_SOURCE`; no forbidden console/sigaction/thread calls.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
Plan 03 documents the stop contract in README and header comments (README not yet updated for `--raise-after`; the --help text is). Plans 04 and 05 add the remaining stop cases.

## Self-Check: PASSED
