---
phase: 09-mmc3
plan: 10
subsystem: tests
tags: [mmc3, blargg, oracle, nightly-lane, fetch]
requires: [09-09]
provides:
  - "tests/mmc3/pins.txt: 12 blargg MMC3 ROMs pinned at 95d8f62 by sha256 and size, licence none-stated"
  - "fetch_blargg_mmc3.cmake: guarded sparse blobless depth-1 fetch with marker .nesturbator-mmc3"
  - "mmc3-oracle (tests/mmc3/oracle.c, oracle.h): $6000 and $F8 result judges"
  - "mmc3-oracle configure/build/test/workflow presets and NESTURBATOR_MMC3_ORACLE"
  - "mmc3.fetch.guard in ci; fetch_guard.cmake SUBDIR, MARKER, PINS parameters"
affects: [09-11, 09-12]
tech-stack:
  added: []
  patterns: ["network lane registered only under an option, rows from a tab table"]
key-files:
  created: [tests/mmc3/pins.txt, tests/mmc3/oracle.txt, tests/mmc3/oracle.h, tests/mmc3/oracle.c, tests/cmake/fetch_blargg_mmc3.cmake, tests/cmake/mmc3_oracle_run.cmake]
  modified: [tests/cmake/fetch_guard.cmake, tests/CMakeLists.txt, CMakePresets.json, README.md]
key-decisions:
  - "mmc3_test_2/1-clocking passes at frame 27; budget set to 120 (27 plus at least 60, rounded up to a multiple of 60)"
  - "Judgement is in oracle.h as pure functions so Plan 11 can test pass, timeout and no-signature offline"
  - "No signature yet is not final in the $6000 loop: the ROM may not have written it, so only the budget decides"
requirements-completed: [BOARD-03]
status: complete
duration: 25 min
completed: 2026-10-10
commits: 2
plan_head_before: e31953675cc5bf652f0a94a61ce620aa46844534
plan_head_after: 2fff748175c58939e238ccd6df62d1e491240749
actuals:
  tokens: 30000
  tasks: 2
  commits: 2
---

# Phase 9 Plan 10: Blargg MMC3 Oracle Lane Summary

A network lane fetches blargg's `mmc3_test_2` and `mmc3_irq_tests` at `95d8f62`, verifies all 12 ROMs by sha256 and size, and judges `mmc3_test_2/1-clocking` as a pass through a test-only oracle; `ci` registers only the offline fetch guard.

## Accomplishments

- `fetch_blargg_mmc3.cmake` mirrors `fetch_vectors.cmake`: marker-only delete, in-tree/symlink/case refusal, stall limits, rev-parse check, exact `.nes` set, `.git` removed.
- `mmc3-oracle` reads the `$6000` protocol through `nesturbator__map_cpu_read` and `$F8` through `nesturbator_peek_cpu_ram`; exit 0 pass, the reported value, 120 timeout, 121 no signature, 122 load, 123 usage; never 77.
- Presets `mmc3-oracle` (configure, build, test with label, `noTestsAction` error, JUnit under `build/mmc3-oracle/`, workflow); `cmake --workflow --preset mmc3-oracle` passes with the `1-clocking` row.
- `fetch_guard.cmake` takes `SUBDIR`, `MARKER`, `PINS` (defaults keep `vectors.fetch.guard` unchanged); `mmc3.fetch.guard` passes offline in `ci`.
- README documents the lane (rule 6).

## Measured budget

`mmc3_test_2/1-clocking` reports pass at frame 27 (the row's budget is 120 frames). Also seen once while checking: `mmc3_irq_tests/1.Clocking` passes at budget 1200 (frame 1260 including the 60 extra); that row is Plan 11's.

## Deviations from Plan

- The `mmc3.fetch.guard` registration and `fetch_guard.cmake` change landed in the Task 1 commit (they share `tests/CMakeLists.txt` with the lane); Task 2's commit holds the README text only.
- [Rule 1 - Bug] The first oracle draft ended the `$6000` loop on "no signature" before the ROM had written it, and printed "reported" for code 120; both fixed before the first commit.

## Known Stubs

None.

## Self-Check: PASSED

`cmake --workflow --preset ci` (443 tests), `--preset hygiene` and `--preset mmc3-oracle` pass; commits 50de12c and 2fff748 exist.
