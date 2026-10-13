---
phase: 14-runner-process-contract
plan: 05
subsystem: runner
tags: [signal, exit-status, unmet-output, movie-fixture]
status: complete

requires:
  - phase: 14-04
    provides: remaining stop cases, runner_stop_case helper
provides:
  - report_unmet() and the "not reached:" stderr line in runner/main.c
  - stop.nmovie (10 frames) and stall.nmovie (4000 frames) in the save fixture
  - runner.stop.unmet_{hash_frame,hash_state,state_at_frame,dump_frame,hash_audio,movie,order}, met_at_stop, late
affects: [14-06, 14-07]

actuals:
  tokens: 9000
  tasks: 2
  commits: 2

plan_head_before: a325d14
plan_head_after: f640598ecb79706919fe0da9762edd2532c4a537

tech-stack:
  added: []
  patterns: ["argv position recorded per request so a stop names the first unmet one"]

key-files:
  created: []
  modified: [runner/main.c, tests/runner/save_rom.c, tests/cmake/runner_stop.cmake, tests/CMakeLists.txt, README.md, .planning/REQUIREMENTS.md]

key-decisions:
  - "Task 1 and Task 2 code landed in one feat commit (save_rom.c with the movie fixtures) because unmet_movie is registered with the Task 1 cases and needs the fixture to pass; README and REQUIREMENTS.md are the second commit"

requirements-completed: [RUN-02, RUN-03]

coverage:
  - id: D1
    description: "A stopped run exits 1 and names the first unmet request in command-line order; met-at-stop and late-flag runs exit 0; no audio hash on a stop"
    requirement: RUN-02
    verification:
      - kind: unit
        ref: "ctest runner.stop.unmet_*, runner.stop.met_at_stop, runner.stop.late"
        status: pass
    human_judgment: false
---

# Phase 14 Plan 05: Unmet outputs on a stopped run Summary

A stopped run now exits 1 and prints `nesturbator-run: not reached: <option>` for the first requested output it did not produce, withholds the audio hash, and leaves open-ended `--frames` runs at 0.

## Accomplishments

- `runner/main.c`: argv index per request, `report_unmet()`, called after the stop line; status 1 only when it was 0. The audio hash was already on the not-stopped branch only (D-09).
- Save fixture writes `stop.nmovie` (56 bytes) and `stall.nmovie` (16016 bytes) with byte-shift little-endian writes.
- Nine CTests added; README `Stopping the runner` and `Exit status` row 1, the main.c header comment and the RUN-02 line in REQUIREMENTS.md state the rule. RUN-02 and RUN-03 checkboxes and traceability left alone.

## Deviations from Plan

- The plan's test counts (18 after Task 1, 19 after Task 2) assumed a baseline of 10; the actual baseline was 11, so `ctest -N -R '^runner\.stop\.'` lists 20. All nine new cases are present.
- Commit split: see key-decisions.
- clang-format reflowed the new main.c comment and macro (applied).

## Verification

- `cmake --workflow --preset ci`: 578 of 578 passed.
- clang-format dry run clean. The asan and hygiene workflows and the GCC 14 docker compile were not run here (hygiene runs on commit; docker not used).

## Known Stubs

None.

## Self-Check: PASSED

- afc6478 and f640598 are on the branch; the six modified files exist; stop.nmovie is 56 bytes and stall.nmovie 16016 bytes.
