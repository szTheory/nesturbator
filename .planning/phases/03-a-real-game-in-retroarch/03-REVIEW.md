---
phase: 03-a-real-game-in-retroarch
reviewed: 2026-10-09T19:43:58Z
depth: standard
files_reviewed: 34
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - include/nesturbator.h
  - libretro/libretro.c
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
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/fuzz_registration.cmake
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_cartridge.c
  - tests/core/test_controller.c
  - tests/core/test_palette.c
  - tests/fuzz/rom_loader.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_render.c
  - tests/ppu/test_sprites.c
  - tests/runner/game_movie.c
  - tests/runner/movie_fixture.h
  - tests/runner/test_movie.c
  - tools/palgen/palgen.c
findings:
  critical: 0
  warning: 1
  info: 0
  total: 1
status: issues_found
---

# Phase 03: Code Review Report

**Reviewed:** 2026-10-09T19:43:58Z
**Depth:** standard
**Files Reviewed:** 34
**Status:** issues_found

## Summary

Reviewed the Phase 03 task-owned source scope reconstructed from the 03-01 through 03-14 summaries and task commit evidence. Evaluation-scope provenance was degraded (`source=phase-range`, `reason=no-reachable-task-commits`, with 12 unreachable plan commit identifiers); its broad fallback also included later-phase files, so those were excluded from this Phase 03 scope. The PPU sprite overflow path does not reproduce the 2C02's diagonal overflow scan and can return incorrect status for real programs.

## Warnings

### WR-01: Sprite overflow scan misses hardware false positives

**File:** `src/ppu.c:75-89`
**Issue:** After the first eight in-range sprites, the evaluator continues testing only each candidate's Y byte (`eval_latch`). The 2C02's overflow behavior advances through OAM bytes diagonally after the eighth sprite; tile, attribute, and X bytes can be interpreted as Y values and falsely set overflow, while the scan advances differently through remaining OAM. Games that poll `$2002` bit 5 therefore receive incorrect results, even though the ordinary eight-sprite boundary works.
**Fix:** Model the post-eighth-sprite `n/m` diagonal scan and its byte-level comparison behavior; add cases where a non-Y OAM byte is in range and where the scan skips candidates, asserting `$2002` bit 5.

---

_Reviewed: 2026-10-09T19:43:58Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
