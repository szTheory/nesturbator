---
phase: 08-mmc1-and-battery-saves
plan: 08
subsystem: libretro
tags: [libretro, retro_memory_save_ram, retroarch-e2e, battery-save, srm]
requires:
  - phase: 08-mmc1-and-battery-saves
    provides: plan 01 (nesturbator_get_memory), plan 06 (runner --save-dir), plan 07 (SXROM ROM, pinned N 400)
provides:
  - retro_get_memory_data/size for RETRO_MEMORY_SAVE_RAM (span, or NULL/0)
  - libretro.host check_save_memory (size, stable pointer, direct-instance bytes, NULL/0 edges)
  - ra_session in run_retroarch.cmake and the two-session save mode
  - "Compare the RetroArch battery save round trip" CI step and artifacts
  - libretro_saves = "true" in the .info
affects: [phase-08-verification, phase-ship]
tech-stack:
  added: []
  patterns: [one factored RetroArch launch with per-tag file names shared by game and save modes]
key-files:
  created: []
  modified:
    - libretro/libretro.c
    - libretro/nesturbator_libretro.info
    - tests/libretro/libretro_host.c
    - tests/retroarch/run_retroarch.cmake
    - tests/retroarch/test.cfg.in
    - .github/workflows/ci.yml
    - README.md
    - include/nesturbator.h
key-decisions:
  - "The adapter fetches the span once in retro_load_game; retro_run and retro_reset never refetch it (D-21)."
  - "Game mode now uses tag game (runner-game.ppm, shot-game.png/.bmp); the CI artifact list follows the new names."
  - "SAVE_FRAME is 400, the pinned N of m1sxrom."
requirements-completed: [SAVE-05]
status: complete
commits: 4
plan_head_before: ce00519314159f160fd306304ef8c8e022ef9fdd
plan_head_after: 13fcec937c5232adf6811357870791aed98697a8
actuals:
  tokens: 45000
  tasks: 3
  commits: 4
completed: 2026-10-10
---

# Phase 8 Plan 08: libretro battery span and two-session RetroArch save round trip Summary

`RETRO_MEMORY_SAVE_RAM` now returns the battery span through the libretro adapter, and the e2e script runs two RetroArch 1.22.2 sessions of the SXROM ROM that write, then load back, a 32,768-byte `.srm`, checked frame by frame against the runner.

## What was built

- **Task 1 (ccb1d51):** `retro_get_memory_data/size` return `save_data`/`save_size`, filled once in `retro_load_game` from `nesturbator_get_memory` and cleared in `retro_unload_game` and `retro_deinit`. Every other id, no game, the test card and a battery-less game give NULL and 0. `.info` says `libretro_saves = "true"` with a description free of phase numbers. `libretro.host` gained `check_save_memory`: size 8192, non-NULL pointer, `RETRO_MEMORY_SYSTEM_RAM` NULL/0, pointer unchanged after 3 frames and after `retro_reset`, span bytes `memcmp`-equal to a direct instance after the same 3 frames, NULL/0 after unload, NULL/0 for a battery-less image and for the test card.
- **Task 2 (2703fe9):** `run_retroarch.cmake` takes exactly one of `-DROM/-DFRAME` or `-DSAVE_ROM/-DSAVE_FRAME`; steps 5-9 are `function(ra_session tag content frame runner_extra)` with per-tag files. Save mode asserts `saves/` empty, runs `s1`, asserts exactly one `<stem>.srm` of 32,768 bytes with `5341564544415441` at 0x100, copies it as the runner's `.sav`, runs `s2` with `--save-dir`, asserts the screenshots differ and re-asserts the single `.srm`. `test.cfg.in` gains the six D-20 keys. `ci.yml` gains the save step and extended artifact paths.
- **Task 3 (b6d33d0, 13fcec9):** README describes `RETRO_MEMORY_SAVE_RAM`, the shared `.srm`/`.sav` bytes, the two-session check and the status line (mappers 0, 1, 2, 3 and 7 with battery saves).

## Deviations from Plan

**1. [Rule 1 - Bug] Header comment tripped abi.float_scan under the nofp preset**
- **Found during:** Task 3 local gate
- **Issue:** `include/nesturbator.h` lines 341 and 346 (added in plans 01/02) said "NES 2.0"; the scanner reads `2.0` as a decimal floating literal, so `abi.float_scan` failed in `nofp`.
- **Fix:** wrote "NES 2" there, as line 240 already does.
- **Commit:** b6d33d0

**2. [Orchestrator instruction over plan] No push, PR or CI watch**
- Task 3 asks for the push, draft PR and a green CI run ID. The spawning prompt says not to push or open PRs, because the orchestrator ships. That part is left to the orchestrator; no CI run ID exists yet for this HEAD.

## Verification

| Check | Result |
|---|---|
| `libretro.host` and the targeted ctest set | pass |
| `cmake --workflow --preset ci` | pass |
| `cmake --workflow --preset asan` | pass (418 tests) |
| `cmake --workflow --preset hygiene` | pass (8 tests) |
| `cmake --workflow --preset nofp` | only `abi.undefined_symbols` fails, the known pre-existing macOS `___stack_chk_*` failure; `abi.float_scan` passes after the fix |
| Local RetroArch 1.22.2 save mode (pinned DMG, sha256 verified, N=400) | pass: s1 and s2 frames equal the runner's, `M1_P512K_CR8K_S32K.srm` written, loaded back, screenshots differ |
| Local RetroArch 1.22.2 game mode (nesteroids, frame 60) | pass |
| CI run on the pushed HEAD (retroarch-e2e, hash-equality, CI required) | not run here: orchestrator ships; this is the remaining gate for SAVE-05 |

`retroarch-e2e` artifact names: `retroarch-e2e-frames` (asset evidence, `runner-game.ppm`, `shot-game.*`, `run-save/runner-s1|s2.ppm`, `shot-s1|s2.png|bmp`, `saves/M1_P512K_CR8K_S32K.srm`).

## Known Stubs

None.

## Threat Flags

None. The e2e keeps the existing before/after snapshot around every session (T-08-13) and the pinned sha256 (T-08-12); distinct per-session names plus the screenshots-differ assertion cover T-08-14.

## Next step for the owner

Push the branch, open the draft PR titled `feat: MMC1 boards and battery saves`, confirm the CI run for the pushed HEAD is green including `retroarch-e2e`, then merge.

## Self-Check: PASSED
