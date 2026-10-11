---
phase: 10-close-out-and-boot-to-play
plan: 04
subsystem: tests
tags: [nes-runner, boot-to-play, mutants, boundary-traces, judge, readme]
requires: ["10-03"]
provides:
  - "eight synthetic mutants and the boundary, adjacency, empty and ordering traces asserted against judge() inside runner.boot_to_play"
  - "README paragraph naming nes-runner v1 as the boot-to-play proof"
affects: [10-05, 10-06, 10-07]
tech-stack:
  added: []
  patterns: ["mutants are in-memory edits of recorded traces, never re-run", "boundary values derived from the named #defines"]
key-files:
  created: []
  modified: [tests/runner/test_boot_to_play.c, README.md]
key-decisions:
  - "Mutants and boundary traces live in run_mutants() in the same executable; no second CTest"
  - "No judge predicate or threshold changed; every case returned its expected mask on the first run"
requirements-completed: []
status: complete
commits: 3
plan_head_before: 09f5755772776e1e966cdcb56815fbe1b0278a50
plan_head_after: 31acd9f1bd7701f2ba7894a7f80fed7cdaa8f9b1
actuals:
  tokens: 9000
  tasks: 3
  commits: 3
duration: 20 min
completed: 2026-10-11
---

# Phase 10 Plan 04: Mutants, boundaries and README Summary

`judge()` is now pinned by eight synthetic broken-core mutants and by both sides of every threshold, all inside the existing `runner.boot_to_play` test, and the README describes the check.

REQUIREMENTS.md still shows PLAY-01 open on purpose: later plans in this phase still declare it.

## Tasks

| Task | Commit | Result |
|------|--------|--------|
| 1 Tracer: silence mutant end to end | 4ec13a7 | `mutant_from` copy helper plus the silence mutant (mask 0x4) |
| 2 TDD: seven more mutants and all boundary traces | c7b708e | 25 cases, each with its expected mask |
| 3 ASan run and README paragraph | 31acd9f | ASan time recorded; README names predicates, controls, mutants, save check |

## Masks observed

Contains-bit mutants: silence 0x4, constant tone 0x4, click then silence 0x4, static picture with ticking counter 0x2, rainbow-only title 0x7, NMI-alive hang 0x1, early death 0x1.
Exact masks: input latched late 0x1; latch at slack 0, past slack 0x1; RAM differs before input 0x1; K 35 0x1 / 36 0; F 4095 0x2 / 4096 0; audio floor 2047 0x4 / 2048 0; A/B share 30719 0x2 / 30720 0; window share 11 of 16 0x4 / 12 of 16 0; plateau ladder 0; equal scores 60 frames apart 0x1; 0 frames and 599 frames 0x7.

## ASan time

`cmake --workflow --preset asan` passed 452 of 452. `runner.boot_to_play` took 52.06 s under ASan (TIMEOUT 300). In the `ci` preset it takes about 3 s.

## TDD record (Task 2)

The cases exercise an already-implemented `judge`, so they passed on first run; there was no separate RED. Each boundary pair asserts the failing side and the passing side with exact masks, so a drifted or disabled predicate fails the test. `judge` was not touched (PROH-10-04-02) and no mutant runs the emulator (PROH-10-04-01).

## Deviations from Plan

None - plan executed as written.

## Verification

`cmake --workflow --preset ci` passed (452 of 452), `cmake --workflow --preset hygiene` passed (8 of 8), `cmake --workflow --preset asan` passed (452 of 452).

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

tests/runner/test_boot_to_play.c and README.md changes present; commits 4ec13a7, c7b708e and 31acd9f are ancestors of HEAD.
