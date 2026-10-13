---
phase: 14-runner-process-contract
plan: 08
subsystem: runner
tags: [ci, timeouts, d-15, signal, draft-pr]
status: complete

requires:
  - phase: 14-07
    provides: draft PR #45, runner.stop.win_console kept
provides:
  - measured TIMEOUTs on runner.signal.* and runner.stop.win_console
  - per-leg hosted timing evidence for the D-15 2000 ms bound
affects: []

actuals:
  tokens: 1000
  tasks: 2
  commits: 2

plan_head_before: 11bff53dac04e28855e924030ed1ecb008b51d64
plan_head_after: 803560fc7468cd0376fd9a01ab816a278f118e3e

tech-stack:
  added: []
  patterns: ["CTest TIMEOUT = twice a measured hosted run, run id in a comment"]

key-files:
  created: []
  modified: [tests/CMakeLists.txt]

key-decisions:
  - "NESTURBATOR_SIGNAL_TIMEOUT 2 s and runner.stop.win_console TIMEOUT 1 s, from CI run 38322038900 (ENGINEERING section 5)"
  - "No D-15 cause fix needed: the 2000 ms bound held on every leg first time; test_signal.c, stop.c and main.c unchanged"

requirements-completed: [RUN-02, RUN-03]

coverage:
  - id: D1
    description: "D-15 signal-to-exit bound of 2000 ms held on every Linux and macOS leg and the asan job"
    requirement: RUN-02
    verification:
      - kind: integration
        ref: "CI run 38322038900 and 38323339041: runner.signal.* Passed on 4 POSIX build legs and asan (helper asserts 2000 ms)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Final pushed head green on every check, six platforms"
    requirement: RUN-02
    verification:
      - kind: integration
        ref: "see Final head below"
        status: pass
    human_judgment: false

duration: 20min
completed: 2026-10-12
---

# Phase 14 Plan 08: Measured timeouts and the final green head Summary

**The signal tests now time out at twice their slowest hosted run (2 s) and runner.stop.win_console at 1 s, the D-15 2000 ms bound held on all four POSIX build legs and asan with no fix needed, and PR #45 stays draft with every check green.**

## Measured wall times (CI run https://github.com/szTheory/nesturbator/actions/runs/38322038900)

Passing runs print no `signal-to-exit` lines (0 in the log); a pass means the helper's own 2000 ms assertion held.

| Leg | term | int | term_exitflush | second | load_error | second_in_flush |
|---|---|---|---|---|---|---|
| ubuntu-24.04 x64 | 0.03 | 0.03 | 0.03 | 0.53 | 0.33 | 0.53 |
| ubuntu-24.04-arm | 0.03 | 0.03 | 0.03 | 0.53 | 0.33 | 0.53 |
| macos-15 arm64 | 0.06 | 0.17 | 0.05 | 0.63 | 0.50 | 0.68 |
| macos-15-intel x64 | 0.10 | 0.18 | 0.18 | 0.65 | 0.38 | 0.58 |
| asan | 0.08 | 0.08 | 0.08 | 0.58 | 0.33 | 0.58 |

Slowest: 0.68 s (macos-15 arm64, second_in_flush), so TIMEOUT 2 s. runner.stop.win_console: windows-2025 0.27 s, windows-11-arm 0.09 s, so TIMEOUT 1 s. The re-run on the first timeout head (run 38323339041) was slower on macos-15 (second 0.69, second_in_flush 0.74, still under the 2 s timeout) and windows-2025 win_console 0.50 s (under 1 s). Risk noted: a 1 s win_console timeout has under 2x margin on that second sample.

## Task Commits

1. **Task 1: measured TIMEOUTs** - `6535069` (test)
2. **Task 1 follow-up** - `803560f` (test): moved the `set(NESTURBATOR_SIGNAL_TIMEOUT ...)` line to column 0 so the plan's verify regex matches it
3. **Task 2: final green head** - no code change; see below

No commit touched tests/runner/test_signal.c, runner/stop.c or runner/main.c. SKIP/DISABLED property count unchanged (0).

## Final head

Run 38323339041 on 6535069 was green on all 14 jobs (six build, asan, two nofp, hygiene, title, hash-equality, retroarch-e2e, `CI required`). The final head is the one pushed after the column-0 move and this SUMMARY; its run is recorded in STATE after it finished. PR: https://github.com/szTheory/nesturbator/pull/45 (draft, not merged).

## Deviations from Plan

**1. [Rule 3 - Blocking] Verify regex needs column 0.** Found during Task 1: the plan's verify uses `^set\(NESTURBATOR_SIGNAL_TIMEOUT`, but Plan 06's line sat indented inside `if(NOT WIN32)`. Fix: removed the two leading spaces (commit 803560f). Verified by the regex matching exactly one line.

**Total deviations:** 1 auto-fixed. **Impact:** none on behaviour.

## Known Stubs

None.

## Self-Check: PASSED

- tests/CMakeLists.txt carries one matching `set(NESTURBATOR_SIGNAL_TIMEOUT 2)` line; 2000 still in tests/runner/test_signal.c; `cmake --workflow --preset ci` 585/585 and hygiene passed locally.
- Commits 6535069 and 803560f are ancestors of HEAD.
