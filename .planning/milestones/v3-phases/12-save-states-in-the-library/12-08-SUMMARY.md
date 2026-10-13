---
phase: 12-save-states-in-the-library
plan: 08
subsystem: runner
tags: [save-states, runner, cli, c17]
status: complete

requires:
  - phase: 12-save-states-in-the-library
    provides: state save/load API and header layout (Plan 01), battery generation on load (Plan 07)
provides:
  - nesturbator-run --state-at-frame N:FILE (repeatable, N may be 0) and --load-state FILE
  - exit status 5 with "cannot load state <path> (<word>)" for rejected states; exit 1 for unreadable files
  - runner.state_mutate test tool and runner.state.* CTest cases
affects: [12-09, 12-10]

requirements-completed: []

actuals:
  tokens: 17000
  tasks: 2
  commits: 2

plan_head_before: 55a29f9b3b666d2a8d0c7837cf45b463900b78e6
plan_head_after: f8ddc9821b6e5fe119955cdceafead32bb9850de

key-files:
  created:
    - runner/state_file.c
    - runner/state_file.h
    - tests/runner/state_mutate.c
    - tests/cmake/runner_state.cmake
  modified:
    - runner/main.c
    - runner/CMakeLists.txt
    - tests/CMakeLists.txt
    - README.md

key-decisions:
  - "State load runs after the battery load and shadow capture, so a state with different battery bytes is written to the .sav at exit (Open Question 2); tested by runner.state.save_dir."
  - "Only the frame number K (header offset 32) is read after a load; ticks (offset 40) is left for Plan 09's range checks, which need it."
  - "The loop starts at K + 1 and states for N == K are written before the loop, so --state-at-frame 0 works with and without a load."

duration: 40 min (plus about 70 min waiting on host stalls)
completed: 2026-10-12
---

# Phase 12 Plan 08: Runner state flags Summary

`nesturbator-run` now writes states with `--state-at-frame N:FILE` through the atomic file replace and resumes from them with `--load-state FILE`, with absolute frame numbers, exit 5 plus a D-07 word for every rejection class and exit 1 for unreadable files.

## Accomplishments

- `--state-at-frame` (repeatable, N of 0 or more) and `--load-state` (once; a second exits 2) in `runner/main.c`; the load follows the battery load (D-10) and the loop continues at K + 1.
- `runner/state_file.c`: bounded read of `state_size + 1` bytes, directory and missing file exit 1 with `cannot read state`.
- `runner.state_mutate` makes truncated, flipped, version-bumped and CRC-valid-but-invalid copies at test time; no state file is committed.
- Ten `runner.state.*` cases plus `runner.usage.load_state.twice`; all pass. Run times: each 0.01 to 0.22 s (roundtrip 0.22 s).
- README "The runner" documents both flags, the D-07 words, the `.sav` behaviour and `5 state rejected` on both exit-status lines.

## Task Commits

1. Task 1 (tracer): `7bc3f67` feat, runner code. Verified end to end by runner.state.roundtrip and runner.state.reject_game.
2. Task 2: `f8ddc98` test, mutate tool, remaining cases, README.

The Task 1 test files (runner_state.cmake, CMake registration) landed in the second commit because the two tasks were verified together; the tracer proof ran before either commit.

## Deviations from Plan

None in behaviour. One test expectation was corrected during Task 2: the fixture .sav begins with `SAVE` then a zero byte at the fifth position (the plan's seed text suggested otherwise), so `save_dir` checks `5341564500`.

## Verification

- `ctest --preset ci -R "^runner\.(state|usage|save)\."`: 40 of 40 passed.
- `cmake --workflow --preset ci`: all tests passed except `core.state_fork.dabg`, `core.state_fork.runner` and `core.state_fork.stress`, which timed out at 300 s with about 170 s of user CPU against 952 s wall (host stall; these tests do not touch the runner). Re-run alone: dabg and runner passed; `core.state_fork.stress` timed out again on two solo runs (wall 990 s and 1050 s, user CPU 60 to 80 s). The final-state of `core.state_fork.stress` is therefore NOT confirmed green on this host in this plan; it was green in Plan 06 and this plan changes no core code. Needs a re-run on an unstalled host.
- The asan workflow was not run (host stalls; orchestrator asked never to overlap with ci).
- No ctest or cmake started by this run is still running.

## Known Stubs

None.

## Threat Flags

None. T-12-08 mitigated (read bounded to state_size + 1), T-12-09 mitigated (writes only through nesturbator_run_file_replace to the given path; idempotent case checks the directory), T-12-10 accepted and documented.

## Next

Plan 09 adds `--hash-state`, K range checks, and the `--movie` and no-ROM refusals. RUN-01 and STATE-03 stay pending.

## Self-Check: PASSED

Created files exist; commits 7bc3f67 and f8ddc98 are ancestors of HEAD.
