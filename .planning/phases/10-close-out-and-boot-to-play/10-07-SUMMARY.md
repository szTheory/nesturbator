---
phase: 10-close-out-and-boot-to-play
plan: 07
subsystem: tests
tags: [libretro, parity, save, asan, readme, close-out]
requires: ["10-06"]
provides:
  - "libretro.host --rom-parity --sav-in FILE and --sav-expect FILE"
  - "holymapperel.<key>.run1.sav_copy with fixture holymapperel_<key>_run1_savcopy"
  - "libretro.parity.m1sxrom.run1/.run2 and libretro.parity.m4tkrom.run1/.run2 (19 parity tests)"
  - "README parity coverage and consistency-versus-correctness statements"
affects: []
tech-stack:
  added: []
  patterns: ["private copy of a file another test rewrites, ordered by a fixture", "one CTest per save run"]
key-files:
  created: []
  modified: [tests/libretro/libretro_host.c, tests/CMakeLists.txt, README.md]
key-decisions:
  - "Run 2's runner dump requires the run-1 save copy fixture, so no parity test can read a save file run 2 is still writing"
  - "ASan timeout left at 10 minutes: twice the estimate is 593 s, under the 600 s threshold"
requirements-completed: [BOARD-04, PLAY-01]
status: complete
commits: 2
plan_head_before: 9fdc777573cf931d477819bc02c337b34c597456
plan_head_after: 6ba0a2707fdfbeaf6f1ecee0a988b063e52ed773
actuals:
  tokens: 12000
  tasks: 2
  commits: 2
duration: 25 min
completed: 2026-10-11
---

# Phase 10 Plan 07: Save round-trip parity and close-out Summary

The libretro adapter's battery span now round-trips through `--sav-in`/`--sav-expect` for both save boards (m1sxrom, m4tkrom) against the runner's `.sav` files, with a private run-1 copy that removes the run1/run2 file race; the README states the parity coverage and that six-platform equality shows consistency, not correctness.

PR title: feat: NMI edge fix, board parity close-out and nes-runner boot-to-play

## Accomplishments

- Task 1 (tracer, commit b90070e): `--sav-in` checks size equals `retro_get_memory_size(RETRO_MEMORY_SAVE_RAM)` and copies into the span after load and before the first frame; `--sav-expect` compares span size and every byte after the last frame, naming the first differing offset. Four tests added: `libretro.parity.<key>.run1/.run2` for m1sxrom and m4tkrom. Total `libretro.parity.*` tests: 19.
- Task 2 (commit 6ba0a27): README coverage text for every committed ROM, the save round trips, the exemptions and the correctness sources (Holy Mapperel codes, AccuracyCoin scoreboard, nightly blargg lane).

## ASan figures

- Local `cmake --workflow --preset asan`: 216 s (3:36 wall), 480 tests passed.
- CI estimate: 254 s x (216 / 185) = 297 s; twice = 593 s, below 600 s, so `timeout-minutes: 10` is unchanged.
- Slowest `libretro.parity.*` under ASan: `libretro.parity.m4tkrom.run2` at 14.21 s (next: nesrunner.play 14.15 s), against `TIMEOUT 120`; below the 60 s threshold, so no TIMEOUT change.

## Phase gate

- `ci`: 480/480 passed. `asan`: 480/480 passed. `hygiene`: 8/8 passed.
- `nofp`: 4/5 locally; `abi.undefined_symbols` fails with `stack_chk_fail;stack_chk_guard` outside the allowlist. This is the macOS Apple clang local toolchain adding stack protector references; the CI `nofp` job uses GCC 14 on Linux (gcc-14 is not installed here). No library file changed in this plan. Not fixed (out of scope); to be confirmed by the CI `nofp` job.

## Verification notes

- The `Total Tests: 19$` grep in the plan's verify cannot be literal: `ctest -N -R` also lists the fixture setup tests it pulls in (47 total). Counting lines matching `: libretro.parity.` gives exactly 19.
- `--repeat until-fail:3 --schedule-random` over the save chain and save parity tests: 22/22 passed.
- Edge checks by hand: a 100-byte `--sav-in` file fails (size mismatch); one flipped byte at offset 5000 in `--sav-expect` fails naming offset 5000.
- PR title matches the ci.yml title job regex; `release-please-config.json` and `.github/workflows/release.yml` unchanged against origin/main.

## Deviations from Plan

**1. [Rule 3 - Blocker] Literal `Total Tests: 19` check replaced by a count of parity test lines**
- **Found during:** Task 1
- **Issue:** fixture setup tests are auto-added to `ctest -N -R` output, so the total is 47.
- **Fix:** counted the `libretro.parity.` lines (19). No file changed.

**Total deviations:** 1 (verification wording only). **Impact:** none.

## Requirements

BOARD-04 and PLAY-01 are marked complete: this is the last plan declaring them, the six-platform hash rows and libretro frame and save parity cover every committed ROM, and the boot-to-play proof and its played-movie parity pass in `ci`.

## Known Stubs

None.

## Self-Check: PASSED

b90070e and 6ba0a27 are in HEAD; modified files exist.
