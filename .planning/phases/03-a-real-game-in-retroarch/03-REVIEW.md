---
phase: 03-a-real-game-in-retroarch
reviewed: 2026-10-09T21:22:35Z
depth: standard
files_reviewed: 5
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - src/internal.h
  - src/ppu.c
  - tests/ppu/test_sprites.c
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
---

# Phase 03: Code Review Report

**Reviewed:** 2026-10-09T21:22:35Z  
**Depth:** standard  
**Files Reviewed:** 5  
**Status:** issues_found

## Summary

Reviewed the Phase 03 sprite overflow implementation, its tests and public documentation. The diagonal byte index stays within primary OAM and the post eighth sprite read and comparison alternate on odd and even dots. One correctness defect remains at the transition into that scan: sprite selection advances six dots too quickly per in range sprite, so the overflow flag becomes CPU visible too early. The new tests encode this incorrect timing.

## Narrative Findings (AI reviewer)

### Critical Issues

### CR-01: First eight sprites are selected six dots too quickly

**Classification:** BLOCKER  
**File:** `src/ppu.c:89-96`  
**Related test:** `tests/ppu/test_sprites.c:106-126`, `tests/ppu/test_sprites.c:146-151`  
**Issue:** On an in range Y comparison, the evaluator copies all four OAM bytes and increments `eval_n` in the same even dot. The next sprite's Y is read on the following odd dot. The [2C02 sprite evaluation sequence](https://www.nesdev.org/wiki/PPU_sprite_evaluation) instead reads and copies the other three bytes on six further dots before advancing to the next sprite. With eight consecutive in range sprites, this implementation compares the ninth Y at dot 82; hardware compares it at dot 130 (first Y at dots 65/66, then eight dots per selected sprite). A CPU read of `$2002` between those dots observes overflow on the emulator when hardware has not yet reached the ninth sprite. The new tests explicitly assert dots 81/82 and 84 for the diagonal path, so they preserve this incorrect boundary even while exercising the post eighth byte walk. This also changes which later OAM entries can be reached before dot 256.

**Fix:** Add per instance copy state for the first eight selected sprites. After an in range Y comparison, read and copy tile, attribute and X on their three subsequent odd/even dot pairs; advance `eval_n` only after the X copy. Keep the existing post eighth `n`/`m` walk once secondary OAM fills. Update the new tests to assert the ninth Y read/compare at dots 129/130 for eight consecutive in range sprites, and derive the following diagonal comparison dots from that boundary. Include a `$2002` observation immediately before and after dot 130.

---

_Reviewed: 2026-10-09T21:22:35Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
