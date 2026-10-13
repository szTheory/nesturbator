---
phase: 14-runner-process-contract
plan: 06
subsystem: runner
tags: [signal, sigterm, sigint, fifo, posix-test-helper]
status: complete

requires:
  - phase: 14-05
    provides: stall.nmovie, runner_save fixture, unmet-output rule
provides:
  - tests/runner/test_signal.c, the deadline-bounded POSIX helper (runner.signal target)
  - runner.signal.{term,int,term_exitflush,second,load_error,second_in_flush} and runner.signal.scratch
  - "signal-to-exit <ms> ms" stderr line for Plan 08
  - EINTR-safe --load-state read in runner/state_file.c
affects: [14-07, 14-08]

actuals:
  tokens: 14000
  tasks: 3
  commits: 4

plan_head_before: 45faef51a54540d6eaa67d1d5b9cb2052a2618a6
plan_head_after: 5b460e5db6c9e43fbe66465492a93665ae1e1f24

tech-stack:
  added: []
  patterns: ["fork/execv helper with a deadline on every wait and SIGKILL-and-reap on failure", "pre-filled stdout pipe for a deterministic stall"]

key-files:
  created: [tests/runner/test_signal.c]
  modified: [tests/CMakeLists.txt, README.md, runner/state_file.c, runner/state_file.h]

key-decisions:
  - "second uses a stdout pipe the helper fills before spawning, not an external stall detector: macOS grows a pipe in 16 KiB steps about 150 ms apart (seconds under ASan), so no short stability window tells a stall from a pause between steps"
  - "An interrupted open or read of the --load-state file is retried in runner/state_file.c"

requirements-completed: [RUN-02]

coverage:
  - id: D1
    description: "External SIGTERM and SIGINT give exit 0 within 2 s, whole save, no temp file; the exit flush alone also fits"
    requirement: RUN-02
    verification:
      - kind: integration
        ref: "ctest runner.signal.term, runner.signal.int, runner.signal.term_exitflush"
        status: pass
    human_judgment: false
  - id: D2
    description: "A later SIGTERM ends a runner stuck on a full stdout pipe, also inside the exit flush; the seed or interval save stays whole"
    requirement: RUN-02
    verification:
      - kind: integration
        ref: "ctest runner.signal.second, runner.signal.second_in_flush"
        status: pass
    human_judgment: false
  - id: D3
    description: "A signal during --load-state only sets the flag; the load error's exit 5 stands"
    requirement: RUN-02
    verification:
      - kind: integration
        ref: "ctest runner.signal.load_error"
        status: pass
    human_judgment: false
---

# Phase 14 Plan 06: External signal tests Summary

**Real SIGTERM and SIGINT from a second process prove exit 0 in under 100 ms with a whole save, a second signal ends a stuck runner, and a signal during loading leaves a load error's exit 5 in charge; the tests found and fixed an EINTR abort of the state read.**

## Performance

- **Tasks:** 3 (plus one deviation fix)
- **Files modified:** 5

## Accomplishments

- `tests/runner/test_signal.c`: fork/execv helper; every wait has a deadline, readiness is a file or FIFO condition, failures SIGKILL and reap the child. Six cases registered under `if(NOT WIN32)` with RUN_SERIAL and a single `NESTURBATOR_SIGNAL_TIMEOUT` line for Plan 08.
- Local `signal-to-exit` (macOS arm64): ci 6 to 7 ms for term, int and term_exitflush; asan 55 to 75 ms.
- Slowest local wall time per test (3 runs each), ci / asan: term 0.03 / 0.33 s, int 0.03 / 0.32 s, term_exitflush 0.03 / 0.36 s, second 0.54 / 0.84 s, load_error 0.34 / 0.61 s, second_in_flush 0.53 / 0.84 s.
- README `Stopping the runner` names the tests.
- Mutation checks done and restored: removing the stop install makes `term` fail in 0.04 s; letting a stop win over a rejected load makes `load_error` fail.

## Task Commits

1. Task 1: helper and `term` - `a29aa6c`
2. Task 2: `int`, `term_exitflush`, `second`, labels, README - `9c188d2`
3. Fix found by Task 3 - `7d21692`
4. Task 3: `load_error`, `second_in_flush`, README - `5b460e5`

## Decisions Made

See key-decisions.

## Deviations from Plan

**1. [Rule 1 - Bug] --load-state open aborted by a stop signal**
- **Found during:** Task 3 (`load_error`, passed alone, failed under the full workflow)
- **Issue:** SIGTERM arriving while the runner was still in `fopen` of the FIFO gave `cannot read state ... (Interrupted system call)` and exit 1, so a stop outranked the load (D-05).
- **Fix:** `runner/state_file.c` retries `fopen` and `fread` on EINTR; header comment updated.
- **Verification:** 60 repeated `load_error` runs and the ci workflow pass.
- **Committed in:** 7d21692

**2. [Plan shape] `second` fills the pipe before spawning**
- The plan's external stall detection (n > 0 and unchanged over three polls) misfired: macOS fills the pipe in steps, and under ASan a step takes over a second, so SIGTERM landed on a running runner. The helper now fills the pipe first and waits for `ready.state` (frame 1, after handler install). The runner blocks in its first stdio flush without a timing assumption. The signal may land just before the block, so the case can exercise the exit flush rather than a mid-loop write; the asserted behaviour (survives the first SIGTERM, dies by a later one, seed save unchanged) is identical.

**3. [Commit split]** The whole helper file was written once, so Task 1's commit already contains the code for all six cases; only `term` was registered there, the rest in Tasks 2 and 3.

**Total deviations:** 1 auto-fixed bug, 2 plan-shape changes.

## Issues Encountered

The GCC 14 docker compile from the plan's verification section was not run (docker not used); Linux behaviour is read in Plan 07's CI.

## Known Stubs

None.

## Next Phase Readiness

Plan 07 (Windows handler tests and CI) can proceed; Plan 08 reads the `signal-to-exit` lines and the timings above to set `NESTURBATOR_SIGNAL_TIMEOUT`.

## Self-Check: PASSED

- Commits a29aa6c, 9c188d2, 7d21692, 5b460e5 are on the branch; tests/runner/test_signal.c exists.
- `cmake --workflow --preset ci` 585 of 585, asan 580 of 580, hygiene passed; clang-format clean.
