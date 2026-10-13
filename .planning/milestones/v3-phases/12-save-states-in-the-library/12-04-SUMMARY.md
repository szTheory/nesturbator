---
phase: 12-save-states-in-the-library
plan: 04
subsystem: core-library
tags: [save-states, tests, fork-matrix]
requires:
  - phase: 12-03
    provides: tests/state_test.h helpers and the STATE-01 contract
provides:
  - CTest core.state_fork, the STATE-03 fork matrix on eight ROMs over six boards
  - st_poison in tests/state_test.h
  - fork_image driver reusable for Plan 05's in-memory image
affects: [12-05 stress image, 12-10 pins]
actuals:
  tokens: 25000
  tasks: 2
  commits: 2
plan_head_before: 08b2e62dc0f7cb63cbff87953bcc208f50af2713
plan_head_after: c8f0dfb05afe40e1f34550ca0324e097614aad6a
key-files:
  created: [tests/core/test_state_fork.c]
  modified: [tests/state_test.h, tests/CMakeLists.txt]
key-decisions:
  - "state_test.h functions are static inline so a test that uses only some helpers builds under -Werror"
  - "Save-every-frame run is kept under --short: asan core.state_fork measured 42 s, far below 120 s, so the fallback was not taken"
requirements-completed: []
status: complete
completed: 2026-10-12
---

# Phase 12 Plan 04: Save-state fork matrix Summary

**Every fork point on eight committed ROMs over six boards loads into a pre-filled, 0xA5-poisoned instance and continues with identical video, audio, ticks, frame numbers and final state bytes; saving every frame changes nothing.**

## Accomplishments

- `st_poison` fills cpu, bus, apu, synth, ppu, mapper.irq, the whole mapper.reg union, PRG-RAM and CHR-RAM with 0xA5, sets derived map.a12, keeps identity, cart, map tables, allocator and host/frame-scoped fields.
- `core.state_fork` (`fork_image`): reference run with per-frame digests, 33 fork points in ci (frames 0 to 30, 60, 150 over 180 frames) and {0, 1, 30} over 60 frames under asan `--short`; load-then-save adjacency check; final state bytes compared; prints `forks <rom> <count>` and fails on a count mismatch.
- ROMs: dabg (NROM), runner and M1_P512K_CR8K_S32K (MMC1), M2 (UxROM), M3 (CNROM), two M4 (MMC3), M7 (AxROM); pre-fill game nesteroids.
- Measured: ci `core.state_fork` 77 s, asan 42 s locally; TIMEOUT 300 s, COST 100. `cmake --workflow --preset ci` (491 tests) and `--preset asan` (486 tests) pass.
- STATE-03 stays pending: the "saving every frame changes no pinned hash" clause is delivered by Plan 10.

## Visitor fixes

None. Every fork matched on the first run, so `src/state.c` was not changed. The poison was not mutation-tested against a deliberately dropped field.

## Deviations from Plan

**[Rule 3 - Blocker] state_test.h helpers made `static inline`.** A second includer (the fork test) left helpers unused and `-Werror -Wunused-function` failed the build. Files: tests/state_test.h. Commit 176944e.

Otherwise the plan ran as written. The save-every-frame pass first skipped `--short` by mistake, then was restored to match the plan (fallback not needed).

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

- FOUND tests/core/test_state_fork.c; commits 176944e and c8f0dfb in HEAD ancestry
- ci and asan workflows green
