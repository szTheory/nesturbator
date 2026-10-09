---
phase: 03-a-real-game-in-retroarch
reviewed: 2026-10-09T14:06:03Z
depth: standard
files_reviewed: 11
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - src/instance.c
  - src/internal.h
  - src/ppu.c
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_render.c
  - tests/ppu/test_sprites.c
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 1
  info: 1
  total: 2
status: issues_found
---

# Phase 03: Code Review Report

**Reviewed:** 2026-10-09T14:06:03Z  
**Depth:** standard  
**Files Reviewed:** 11  
**Status:** issues_found

## Summary

Reviewed the 11 paths in the incremental Phase 03-14 scope. The row-zero sprite preparation is consistently wired from pre-render initialization through evaluation, fetch, and caller-buffer output. The public header still contains a contradictory description of frame execution, and `src/ppu.c` repeats two hardware comments.

## Warnings

### WR-01: Public API documentation says the CPU does not run during frames

**Severity:** WARNING  
**File:** `include/nesturbator.h:214-218`  
**Issue:** The `nesturbator_create` comment says “The CPU does not yet run during frames,” but the library executes CPU instructions during `nesturbator_run_frame` whenever a cartridge is loaded. This gives API consumers a materially false description of the current core and conflicts with the documented cartridge-loading and frame-running behavior below it.
**Fix:** Remove the stale sentence and describe the two cases accurately, for example: “With a loaded cartridge, frame calls execute the CPU and connected devices; without a cartridge, each frame is a fixed test pattern and silence.”

## Info

### IN-01: Duplicate PPU hardware comments

**Severity:** INFO  
**File:** `src/ppu.c:234-237,252-253`  
**Issue:** The vblank timing comment is repeated verbatim at lines 234-237, and the odd-frame dot-skip comment is repeated at lines 252-253. The duplicates add noise around timing-sensitive code and can drift independently during later edits.
**Fix:** Keep one copy of each comment immediately before its corresponding condition.

---

_Reviewed: 2026-10-09T14:06:03Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
