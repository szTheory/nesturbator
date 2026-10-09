---
phase: 03-a-real-game-in-retroarch
reviewed: 2026-10-09T13:26:44Z
depth: standard
files_reviewed: 40
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - runner/CMakeLists.txt
  - runner/main.c
  - runner/movie.c
  - runner/movie.h
  - src/bus.c
  - src/cartridge.c
  - src/cpu.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/palette_ntsc.c
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/fuzz_registration.cmake
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_cartridge.c
  - tests/core/test_palette.c
  - tests/fuzz/rom_loader.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_registers.c
  - tests/retroarch/run_retroarch.cmake
  - tests/roms/manifest.txt
  - tests/runner/game_movie.c
  - tests/runner/hashes.txt
  - tests/runner/movie_fixture.h
  - tests/runner/test_movie.c
  - tests/test_process.h
  - tools/palgen/palgen.c
findings:
  critical: 3
  warning: 0
  info: 0
  total: 3
status: issues_found
---

# Phase 03: Code Review Report

**Reviewed:** 2026-10-09T13:26:44Z  
**Depth:** standard  
**Files Reviewed:** 40  
**Status:** issues_found

## Summary

Reviewed the requested Phase 03 source scope at standard depth, excluding Phase 04 files and binary ROM/corpus data. The libretro adapter drops audio for frontends that provide only the per-sample callback, its reset entry point does nothing for a loaded game, and the PPU never prepares sprites for scanline 0. These defects affect normal gameplay and host compatibility.

## Critical Issues

### CR-01: Per-sample audio callback is never used

**Severity:** BLOCKER  
**File:** `libretro/libretro.c:58-62, 206-208`  
**Issue:** The adapter stores `audio_cb` but never calls it. If a frontend supplies `retro_set_audio_sample` without a batch callback, each successful frame produces no audio even though the core generated PCM. The comment says only one of the callbacks should be used, but the implementation only supports the batch variant.
**Fix:** Prefer `audio_batch_cb` when non-NULL; otherwise call `audio_cb(stereo[2*i], stereo[2*i+1])` for each generated sample. Add a host test with only the per-sample callback installed.

### CR-02: Libretro reset leaves a running cartridge untouched

**Severity:** BLOCKER  
**File:** `libretro/libretro.c:151-154`  
**Issue:** `retro_reset()` is empty. Once a cartridge is loaded, RetroArch's reset command therefore leaves CPU, PPU, bus, and audio state at their current execution point instead of performing the soft reset requested by the frontend. The test-card-only comment does not apply to the supported loaded-game path.
**Fix:** Implement an internal soft-reset operation for a loaded instance and call it here, preserving cartridge ROM while resetting CPU/PPU/APU/bus state and fetching the reset vector. Cover reset after the CPU has advanced in the libretro host test.

### CR-03: Sprite rendering omits scanline 0

**Severity:** BLOCKER  
**File:** `src/ppu.c:47-51, 96-103, 214-269`  
**Issue:** Sprite evaluation and fetch are performed for a visible scanline to populate sprites for the *next* scanline (`eval_target = scanline + 1`, and fetch computes `target = scanline + 1`). Evaluation returns immediately for pre-render scanline 261, so no sprite set is prepared for visible scanline 0. At startup, the PPU also begins at scanline 0 without a pre-render pass. Consequently sprites at the top visible row are absent on every rendered frame.
**Fix:** Perform the pre-render evaluation/fetch for target scanline 0 (including NES Y-coordinate wrap semantics), and initialize/reset the PPU so a frame begins with the appropriate pre-render state. Add a test that runs through a full frame boundary and checks a sprite on row 0.

## Warnings

## Info

---

_Reviewed: 2026-10-09T13:26:44Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
