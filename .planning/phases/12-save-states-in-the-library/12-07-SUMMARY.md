---
phase: 12-save-states-in-the-library
plan: 07
subsystem: core-library
tags: [save-states, battery, save-generation, c17]
status: complete

requires:
  - phase: 12-save-states-in-the-library
    provides: load pipeline (Plan 01), rejection sweeps (Plan 06)
provides:
  - nesturbator_load_state raises nesturbator_save_generation once when the battery span changes
  - core.state_generation, the STATE-05 test including the 600-load case
affects: [12-verification]

requirements-completed: [STATE-05]

actuals:
  tokens: 14000
  tasks: 2
  commits: 2

plan_head_before: ca341854e7c027a694c35bf051a4c9acf034009d
plan_head_after: ed340f10ff7934bde63cc02ce48ed9d72fd80096

key-files:
  created:
    - tests/core/test_state_generation.c
  modified:
    - src/state.c
    - tests/CMakeLists.txt
    - include/nesturbator.h
    - README.md

key-decisions:
  - "The span compare (memcmp of cart.save against the state's PRG-RAM bytes at the same offset) runs after VALIDATE and before the copy; the increment is the last step of APPLY, so a refused load cannot move the counter."

duration: 25 min
completed: 2026-10-12
---

# Phase 12 Plan 07: Battery generation on state load Summary

A state load raises `nesturbator_save_generation` exactly once when the state's battery span differs from the instance's, and never otherwise (STATE-05).

## Accomplishments

- `src/state.c`: span compare before the PRG-RAM copy, one increment at the end of APPLY; guarded by `save_size != 0`.
- `core.state_generation` covers: changed span counts once and a repeat counts nothing; 600 loads of a matching state leave the generation unchanged after every load; work RAM outside the span does not count; battery-less NROM never counts; a checksum-refused load never counts; a changed-span load after the instance's own writes counts up from there.
- `include/nesturbator.h` (`nesturbator_get_memory`, `nesturbator_save_generation` comments) and README ("Battery saves", "Save states") state the rule. The sentence "Rewinding past an in-game save reverts the save RAM." is kept.

## Task Commits

1. Task 1 (tracer): 0a21815 feat(12-07): raise the save generation once when a state load changes the battery span
2. Task 2: ed340f1 test(12-07): STATE-05 cases, 600-load case, header and README text

## Verification

- RED shown for the tracer by disabling the increment (core.state_generation failed), then restored. The implementation was written before the test was first run, so the red run was done after the fact rather than first.
- `cmake --workflow --preset ci`: passed on the first full run.
- `cmake --workflow --preset asan`: passed except 4 timeouts (core.state_fork.stress, core.state_reject, core.state_fork.dabg, core.state_fork.runner) at about 1045 s wall on a heavily loaded host with a second ci workflow running at once. Re-run alone under asan: core.state_generation, all core.state_fork.* and core.state_reject passed (4 to 18 s).
- A second ci workflow run hit one timeout, runner.save.interval (921 s wall, same load); alone it passed in 4 s.

## Deviations from Plan

None - plan executed exactly as written (the RED ordering note above aside).

## Known Stubs

None.

## Self-Check: PASSED
