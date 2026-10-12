---
phase: 14-runner-process-contract
plan: 07
subsystem: runner
tags: [windows, console-ctrl-handler, ctrl-break, ci, draft-pr]
status: complete

requires:
  - phase: 14-06
    provides: runner.signal.* POSIX tests, runner_save fixture
provides:
  - tests/runner/test_stop_win.c (runner.stop.win_handler)
  - tests/runner/test_console.c (runner.stop.win_console, kept by evidence)
  - Draft PR #45 with six green build jobs on the last pushed head
affects: [14-08]

actuals:
  tokens: 9000
  tasks: 2
  commits: 3

plan_head_before: 5770d1b6dae337fb564cc38af0209f8337855dba
plan_head_after: 89a32c20f729c8c2547ab2bf5a0517efe5e6ab3f

tech-stack:
  added: []
  patterns: ["Windows handler called directly through stop_internal.h", "FreeConsole, AttachConsole, TRUE handler, CTRL_BREAK_EVENT to group 0 sender"]

key-files:
  created: [tests/runner/test_stop_win.c, tests/runner/test_console.c]
  modified: [tests/CMakeLists.txt, README.md]

key-decisions:
  - "D-16 decided from CI: runner.stop.win_console passed on both hosted Windows legs on its first run, so it is kept"
  - "runner/stop.c and runner/main.c needed no change: MSVC raised no warning and no Windows stop defect appeared"

requirements-completed: [RUN-02, RUN-03]

coverage:
  - id: D1
    description: "The Windows console handler: Ctrl+C and Ctrl+Break handled once then declined, logoff and shutdown declined, close event held until done, the cap or the grace period"
    requirement: RUN-02
    verification:
      - kind: unit
        ref: "ctest runner.stop.win_handler (both Windows legs, run 38320256818)"
        status: pass
    human_judgment: false
  - id: D2
    description: "A real Ctrl+Break sent through the runner's own console gives exit 0, a whole battery.sav and no temp file on hosted Windows"
    requirement: RUN-02
    verification:
      - kind: e2e
        ref: "ctest runner.stop.win_console (windows-2025 0.35 s, windows-11-arm 0.10 s)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Stop, signal and exit-status tests green on all six platforms for one pushed head"
    requirement: RUN-02
    verification:
      - kind: integration
        ref: "CI run 38321279610, six build jobs success; runner.stop.frame_exact.break/int and runner.stop.double on Windows, runner.signal.* on Linux and macOS"
        status: pass
    human_judgment: false

duration: 75min
completed: 2026-10-12
---

# Phase 14 Plan 07: Windows handler and console tests, draft PR Summary

**The Windows console handler is tested directly, a real Ctrl+Break through the runner's own console passes on both hosted Windows legs and is kept, and draft PR #45 shows every stop and signal test green on six platforms.**

## Performance

- **Tasks:** 2
- **Files modified:** 4 (two new test files, tests/CMakeLists.txt, README.md)

## Accomplishments

- `runner.stop.win_handler` (`tests/runner/test_stop_win.c`): Ctrl+C and Ctrl+Break return TRUE then FALSE, logoff and shutdown return FALSE without setting the flag, a close event on a second thread returns only after `nesturbator_run_stop_done()` and inside the cap, returns at the cap when done never comes, and waits the grace period after done. Passed on windows-2025 and windows-11-arm (0.90 s each).
- `runner.stop.win_console` (`tests/runner/test_console.c`): `CREATE_NEW_CONSOLE` runner, wait for a whole 8192-byte battery.sav (no sleep), `FreeConsole`, `AttachConsole` retry, handler registered after attaching, `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0)`. Requires exit 0, whole save starting `SAVE`, no `.tmp`.
- Draft PR #45 `feat: safe runner stop and one exit-status table`, https://github.com/szTheory/nesturbator/pull/45, not merged and not marked ready.
- README `### Stopping the runner` states what the two Windows tests show.

## D-16 outcome: kept

`runner.stop.win_console` passed on both Windows legs of run https://github.com/szTheory/nesturbator/actions/runs/38320256818 : windows-2025 x64 Passed 0.35 s, windows-11-arm arm64 Passed 0.10 s. No API failed, so there is no GetLastError text to record. The test never skips and no SKIP or DISABLED property was added (count is 0 before and after).

## CI runs used

| Head | Run | Result |
|---|---|---|
| 2565d60 (win_handler) | https://github.com/szTheory/nesturbator/actions/runs/38319021957 | six build jobs green; win_handler, frame_exact.break/int, double Passed on both Windows legs; runner.signal.{term,int,term_exitflush,second,load_error,second_in_flush} Passed on the four POSIX legs |
| e7baa43 (win_console) | https://github.com/szTheory/nesturbator/actions/runs/38320256818 | all jobs green, `CI required` included |
| 89a32c2 (README, last code head) | https://github.com/szTheory/nesturbator/actions/runs/38321279610 | six build jobs green, run success |

## Task Commits

1. **Task 1: Windows handler unit test, push, draft PR** - `2565d60` (test)
2. **Task 2: real Ctrl+Break through the console** - `e7baa43` (test); README `89a32c2` (docs)

**Plan metadata:** the commit holding this file (docs: complete plan)

## Files Created/Modified

- `tests/runner/test_stop_win.c` - direct handler cases through `stop_internal.h`
- `tests/runner/test_console.c` - end-to-end Ctrl+Break sender with named-API failure reports
- `tests/CMakeLists.txt` - both registered under `if(WIN32)`; win_console requires `runner_save`, TIMEOUT 60
- `README.md` - the two Windows tests added to the stop section

## Decisions Made

- Keep `runner.stop.win_console` (D-16, evidence above).
- No change to `runner/stop.c` or `runner/main.c`: the first hosted MSVC build of the Windows half of stop.c compiled warning-free and its tests passed.

## Deviations from Plan

None - plan executed exactly as written. Local checks before the first push: `cmake --workflow --preset ci` (585 tests) and `hygiene` passed, GCC 14 compile of the listed files exited 0, clang-format clean. The test files cannot be compiled on this Mac, so the first hosted run was their first compile; it was warning-free.

## Issues Encountered

`gh run view --log` returns nothing while any job in the run is still running (asan), so the per-test lines were read once the run completed.

## Known Stubs

None.

## Next Phase Readiness

Plan 08 measures the timeouts and takes the final head to green on every check; PR #45 stays draft. `main` had not moved past the branch base (0 commits behind origin/main at the start of this plan).

## Self-Check: PASSED

- tests/runner/test_stop_win.c and tests/runner/test_console.c exist; commits 2565d60, e7baa43, 89a32c2 are ancestors of HEAD.
- `gh pr view 45`: draft, title as required; one PR for the branch; author emails end in `@users.noreply.github.com`.
