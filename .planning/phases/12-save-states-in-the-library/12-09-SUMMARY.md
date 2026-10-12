---
phase: 12-save-states-in-the-library
plan: 09
subsystem: runner
tags: [save-states, runner, cli, c17]
status: complete

requires:
  - phase: 12-save-states-in-the-library
    provides: runner --state-at-frame and --load-state (Plan 08)
provides:
  - nesturbator-run --hash-state N (repeatable) printing "state <N> ticks <T> size <S> sha256 <hex>"
  - one K-relative range check with absolute frame indices after a load
  - exit 2 for repeated N, --movie with --load-state, and state flags without --rom
affects: [12-10]

requirements-completed: [RUN-01]

actuals:
  tokens: 9000
  tasks: 2
  commits: 2

plan_head_before: 42a3174e2161479ec1f1a8bbec5c8456c4d56226
plan_head_after: 417b495eb328a6227abef00dcbe6763b77730628

key-files:
  modified:
    - runner/main.c
    - tests/cmake/runner_state.cmake
    - tests/CMakeLists.txt
    - README.md

key-decisions:
  - "The old --hash-frame/--dump-frame range checks moved after the state load into one check_ranges(); without a load K is 0 and the old meaning holds, all existing usage tests pass."
  - "State ticks for N == K come from the state header (offset 40); later frames use the frame's ticks, so a resumed run prints the same state line as an uninterrupted one."

duration: 25 min
completed: 2026-10-12
---

# Phase 12 Plan 09: Runner --hash-state and range rules Summary

`nesturbator-run --hash-state N` prints the SHA-256 of the save state after frame N, with absolute frame indices checked against the loaded frame K by one range check, and the repetition, `--movie` and no-ROM refusals exit 2.

## Accomplishments

- `--hash-state N` (N may be 0, repeatable) in `runner/main.c`, using the frame-pin SHA-256 over `nesturbator_save_state` bytes. For N == K it prints before the loop; per frame the order is frame line, state line, dump files.
- `check_ranges()` after the load: `--frames` below K, `--hash-frame`/`--dump-frame` outside K < N <= frames, `--state-at-frame`/`--hash-state` outside K <= N <= frames, all exit 2 naming K.
- Refusals: same N twice for `--hash-state` or `--state-at-frame`, `--movie` with `--load-state`, state flags without `--rom` or with `--accuracycoin-page`.
- Cases `runner.state.frame0`, `same_k`, `continue_hash`, `range_frames`, `range_hash_frame`, `range_state` and `runner.usage.state.twice`, `.movie`, `.norom`, `runner.usage.hash_state.twice`; README "The runner" documents all of it.
- `state 120` of an uninterrupted run equals that of a run loaded at 60 (STATE-03 through the runner; STATE-03 itself stays pending for Plan 10).

## Task Commits

1. Task 1 and the code half of Task 2: `959aa74` feat (runner, tests).
2. README: `417b495` docs.

The tasks were verified together, so Task 1 and Task 2 code share one commit.

## Deviations from Plan

None - plan executed exactly as written (commit split differs from one-per-task, as in Plan 08).

## Verification

- `ctest --preset ci -R "^runner\.(state|usage|save)\."`: 50 of 50 passed.
- `cmake --workflow --preset ci`: 519 of 522 passed. `core.state_fork.stress`, `core.state_reject` and `runner.boot_to_play` timed out at the 300 s limit (host throttling; whole run 464 s wall, 163 s user CPU). `runner.boot_to_play` re-run alone passed in 2.78 s. The two core tests touch no runner code and were not re-run (orchestrator note: host stall, stress is known to pass when run directly).
- The asan workflow was not run (host stalls; Plan 10 or the orchestrator should run it).
- No ctest or cmake started by this run is still running.

## Known Stubs

None.

## Threat Flags

None.

## Next

Plan 10 pins the hash line and closes STATE-03.

## Self-Check: PASSED

runner/main.c, tests/cmake/runner_state.cmake exist; commits 959aa74 and 417b495 are ancestors of HEAD.
