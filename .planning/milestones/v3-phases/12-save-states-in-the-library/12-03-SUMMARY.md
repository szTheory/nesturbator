---
phase: 12-save-states-in-the-library
plan: 03
subsystem: core-library
tags: [save-states, readme, tests, header-bindings]
requires:
  - phase: 12-01
    provides: the three state functions, the status enum and the public header comments
  - phase: 12-02
    provides: board registers in the state
provides:
  - README "Save states" section, format-version table and "Status codes" table
  - CTest state.status_table (README table equals enum nesturbator_status, section and rewind sentence present)
  - C17 and C++ header bindings for the three state functions
  - tests/state_test.h shared helpers and st_counting_allocator for Plans 04 to 06
  - core.state contract cases for STATE-01 and D-06
affects: [12-04 fork matrix, 12-05, 12-06 rejection sweeps]
actuals:
  tokens: 40000
  tasks: 2
  commits: 2
plan_head_before: 81b02f489d52bdb6a2c452db9f5a6b19b9fd00fc
plan_head_after: 0ec97b3bc66667786b5348017ff84c0225643c02
key-files:
  created: [tests/cmake/status_table.cmake, tests/state_test.h]
  modified: [README.md, tests/CMakeLists.txt, tests/header/header_c.c, tests/header/header_cxx.cpp, tests/core/test_state.c]
key-decisions:
  - "Status table checker is a small -P script (tests/cmake/status_table.cmake) comparing sorted n:NAME pairs, no existing exit-code checker to copy"
  - "The JAM direction tests use an image that loops on CPU RAM $10 and runs JAM when it is non-zero, so a clean state of the same image keeps running"
requirements-completed: [STATE-01]
status: complete
completed: 2026-10-12
---

# Phase 12 Plan 03: Save states README, status table and STATE-01 contract Summary

**README documents save states and every status code, a CTest checks that table against the enum, the state API compiles as C17 and C++, and `core.state` pins size, no-allocation, buffer, idempotency, two-instance, JAM and reset behaviour.**

## Accomplishments

- README: the three functions in the public list, a "Save states" subsection (size/save/load use, check order, header table, frame number and tick continuation, format refusal and the behaviour-revision rule, format-version table, `feat:` bump rule, `NESTURBATOR_ABI_VERSION` unchanged, "Rewinding past an in-game save reverts the save RAM.") and a "Status codes" table for values 0 to 12.
- `state.status_table` fails naming the first difference; checked by temporarily renumbering a row (fails with "lacks 12:NESTURBATOR_ERR_STATE_INVALID"), then restored.
- `header_c.c` and `header_cxx.cpp` bind the three functions and assert `NESTURBATOR_STATE_FORMAT_VERSION == 1` and `NESTURBATOR_ERR_STATE_INVALID == 12`.
- `tests/state_test.h`: the `st_*` helpers moved unchanged, plus `st_counting_allocator` (counting alloc/free and `st_make_instance_counting`).
- `core.state` cases: `size_shape`, `save_never_allocates`, `save_const`, `buffer_rules`, `idempotent`, `two_instances`, `jam_both_ways`, `after_reset`.
- Verification: `cmake --workflow --preset ci` and `cmake --workflow --preset asan` passed (485 tests each run).

## Reset case

The state saved right after `nesturbator_reset` loaded and the next five frames matched on the first run. No field was added to the state; the cause check was not needed.

## Deviations from Plan

None to behaviour. Task 2 was TDD-marked, but Plan 01 and 02 code already satisfied every case, so the cases passed once written (after fixing two test-side mistakes: the JAM loop offset, and the first JAM design needing a clean state that does not re-jam). No change to `src/state.c` was needed, so it is not in the modified list.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

- FOUND commits 658385c and 0ec97b3 in HEAD ancestry
- FOUND tests/cmake/status_table.cmake and tests/state_test.h
- ci and asan workflows green
