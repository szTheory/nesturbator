---
phase: 14-runner-process-contract
plan: 03
subsystem: runner
tags: [readme, docs, windows-launcher, raise-after, ctest]

requires:
  - phase: 14-02
    provides: stop module, --raise-after, --help line
provides:
  - README "Stopping the runner" with the five guarded sentences and the two-case Windows launcher recipe
  - README --raise-after option entry and exit-status row 1 wording
  - runner.exit.table README guards; runner.exit.help --raise-after check
  - runner/main.c header paragraph on stopping
affects: [14-04, 14-05, 14-06, 14-07]

actuals:
  tokens: 5000
  tasks: 2
  commits: 2

plan_head_before: d6a44a7afa3178460be17177c73654aae3d1533f
plan_head_after: 3720a1b92f89dbb2311db51bec818a10c19a1e31

tech-stack:
  added: []
  patterns: ["README sentences guarded by string(FIND) in a ctest script"]

key-files:
  created: []
  modified: [README.md, tests/cmake/runner_exit_table.cmake, runner/main.c]

key-decisions:
  - "The option-list guard matches the bullet text, not the usage code block, so it cannot be satisfied by the synopsis alone"

requirements-completed: [RUN-02, RUN-03]

coverage:
  - id: D1
    description: "README states the stop contract and Windows launcher recipe; loss of a sentence or the --raise-after entry fails ctest"
    requirement: RUN-02
    verification:
      - kind: unit
        ref: "ctest runner.exit.table, runner.exit.help"
        status: pass
    human_judgment: false

duration: 8min
completed: 2026-10-12
status: complete
---

# Phase 14 Plan 03: Stop contract documentation Summary

**README `### Stopping the runner` with a two-case Windows launcher recipe and `--raise-after`, guarded by runner.exit.table, plus the matching runner/main.c header paragraph.**

## Accomplishments
- README section with the five verbatim guarded sentences, the stop line on stderr, load-time stops, atomic save, and both launcher cases (shared console with CREATE_NEW_PROCESS_GROUP; separate console with AttachConsole, own TRUE CTRL_BREAK handler, then group 0).
- `--raise-after N[:SIG[:K]]` in the README synopsis and option list with per-platform defaults; exit-status row 1 names a failed stop save.
- `runner.exit.table` fails on a missing section, sentence, or option entry; `runner.exit.help` fails if `--help` drops the `--raise-after` line.
- runner/main.c header cites POSIX `signal`, `HandlerRoutine`, `SetConsoleCtrlHandler` and states that `--accuracycoin-page` installs no stop handlers.

## Task Commits
1. Task 1: README and guards - `24ea5dc` (docs)
2. Task 2: runner/main.c header - `3720a1b` (docs)

## Deviations from Plan
None - plan executed exactly as written. The guard on the option-list entry was tightened to the bullet text after a local negative check showed the synopsis code block alone satisfied the looser match.

## Verification
- `cmake --workflow --preset ci`: 554/554 passed; hygiene workflow passed; clang-format 18 dry run clean.
- Negative checks: removing the console-close sentence and the `--raise-after` bullet each fail runner.exit.table (restored).
- No forbidden console/thread call followed by a parenthesis in runner/stop.c or runner/main.c. runner/stop.h already agreed with the README, so it is unchanged.

## Next Phase Readiness
Ready for 14-04.

## Self-Check: PASSED
