---
phase: 09-mmc3
plan: 07
subsystem: runner
tags: [mmc3, runner, rejection-messages, ctest]
requires: [09-03, 09-04, 09-05]
provides:
  - "describe_rejection in runner/main.c: four-screen, MMC6, unsupported-mapper and generic stderr messages, exit 1"
  - "EXPECT_ERR stderr substring check in expect_output.cmake and nesturbator_runner_test"
  - "runner.reject.four_screen, .mmc6, .mapper, .generic, .truncated CTests"
affects: [09-12]
tech-stack:
  added: []
  patterns: ["runner reads the 16 header bytes itself; no public rejection-reason API"]
key-files:
  created: []
  modified: [runner/main.c, tests/cmake/expect_output.cmake, tests/CMakeLists.txt, README.md]
key-decisions:
  - "Generic line keeps its wording with the mapper list updated to 0, 1, 2, 3, 4 and 7"
requirements-completed: [MAP-03]
status: complete
duration: 10 min
completed: 2026-10-10
commits: 1
plan_head_before: 9d513ea6e9b38db577f2c0da382b75268f4f3437
plan_head_after: aea5344
actuals:
  tokens: 4500
  tasks: 2
  commits: 1
---

# Phase 9 Plan 07: Runner rejection messages Summary

`nesturbator-run` now names why a cartridge was refused (four-screen, MMC6, unsupported mapper N, or the generic line listing mappers 0, 1, 2, 3, 4 and 7) and exits 1, each proven by a CTest on a committed fuzz seed.

## What was built

- `describe_rejection` guards `len >= 16` and the `NES\x1a` magic, decodes mapper and submapper with the NES 2.0 bit rule, and prints in D-15 order. `free(bytes)` now runs after the message.
- `EXPECT_ERR` is an optional stderr substring check; existing runner tests are unchanged.
- Five `runner.reject.*` tests on seeds `mmc3-four-screen`, `mmc3-submapper-1`, `mapper-118`, `mmc3-prg-8k` and `truncated-header`.
- Runner top comment, `usage()` and README name mappers 0, 1, 2, 3, 4 and 7 and the three messages.

## Deviations from Plan

- Both tasks landed in one commit (aea5344) because they touch the same files and the tracer was verified by the full ci workflow before committing. No behavioural deviation.
- The 0-byte `empty` seed is rejected earlier by the runner's file-read path ("cannot read cartridge", exit 1), not by describe_rejection; the truncated-header test covers the generic line. Exit status 1 holds for both.

## Verification

`cmake --workflow --preset ci` passed (all tests, including the five new ones); `cmake --workflow --preset hygiene` passed. The asan run belongs to Plan 12's phase gate.

## Known Stubs

None.

## Self-Check: PASSED
