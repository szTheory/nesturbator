---
phase: 10-close-out-and-boot-to-play
plan: 06
subsystem: tests
tags: [libretro, parity, holymapperel, nes-runner, ctest]
requires: ["10-03", "10-05"]
provides:
  - "libretro.host --rom-parity <module> <rom> <frames> <ppm> [--movie FILE] [--testcard PPM]"
  - "runner.dump.<key> fixtures (rundump_<key>) writing P6 images under tests/parity"
  - "15 libretro.parity.<key> CTests"
affects: [10-07]
tech-stack:
  added: []
  patterns: ["one CTest per ROM key", "runner dump as fixture so the runner runs once per key", "sentinel fill plus video_calls anti-vacuity checks"]
key-files:
  created: []
  modified: [tests/libretro/libretro_host.c, tests/CMakeLists.txt]
key-decisions:
  - "The not-the-test-card check compares against the runner's test-card image (testframe_ppm) rather than a native-pixel hash"
  - "Option parser is a simple loop so Plan 07 can add --sav-in and --sav-expect"
requirements-completed: []
status: complete
commits: 2
plan_head_before: 17b432d3c6cbd1a7b9ad49af02e36d9ea260e065
plan_head_after: a00984bd9492728972421b2ae55921478284da57
actuals:
  tokens: 14000
  tasks: 2
  commits: 2
duration: 15 min
completed: 2026-10-11
---

# Phase 10 Plan 06: Libretro parity per ROM Summary

`libretro.host --rom-parity` loads a whole ROM through the built libretro module, feeds an optional movie as joypad state, and requires the last frame to equal the runner's P6 image; 15 per-key `libretro.parity.*` CTests now cover the games, nes-runner boot and played movie, and every non-save Holy Mapperel ROM.

## What was built

- Task 1 (tracer, commit 1397f16): `--rom-parity` dispatch ahead of the six-argument mode (unchanged), with sentinel fill before each `retro_run`, `video_calls == frames`, exact-header pixel compare via `compare_with_ppm`, and `--testcard` rejection of a test-card frame. `runner/movie.c` is linked into `libretro.host`. Proven on nesteroids.
- Task 2 (commit a00984b): loops over `NESTURBATOR_HASH_GAMES` register `runner.dump.<key>` plus `libretro.parity.<key>` at frame 180 (nesteroids, dabg, rhde, nesrunner.boot); `nesrunner.play` replays `nesrunner-pos.nmovie` for 600 frames in both runner and module; the Holy Mapperel loop adds `libretro.parity.<key>` at each N for the 10 non-save keys (derived m4w8k included). Save keys carry a comment pointing to Plan 07. Existing empty-list `FATAL_ERROR` stops cover the source lists.

## Verification

- `ctest -N` lists exactly 15 `libretro.parity.*` tests; `libretro.parity.nesrunner.(boot|play)` is 2.
- `cmake --workflow --preset ci`: 474/474 passed. `cmake --workflow --preset hygiene`: passed.

## Deviations from Plan

- The `libretro_host.c` include of the movie header is `../../runner/movie.h` rather than a new include directory (existing include path holds only tests/runner). No behavior change.

**Total deviations:** 1 minor build detail.

## Known Stubs

None.

## Notes

BOARD-04 and PLAY-01 stay open in REQUIREMENTS.md on purpose: Plan 07 still declares them. README and public header are unchanged because this plan changes tests only.

## Self-Check: PASSED

Files modified exist; commits 1397f16 and a00984b are on the branch.
