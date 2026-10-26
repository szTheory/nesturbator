---
phase: 19-jam-reporting-and-sprite-0-hit
reviewed: 2026-10-25T00:00:00Z
depth: standard
files_reviewed: 18
files_reviewed_list:
  - include/nesturbator.h
  - libretro/libretro.c
  - runner/accuracycoin.c
  - runner/accuracycoin.h
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/accuracy/test_conformance.c
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_driver.cmake
  - tests/cmake/accuracycoin_jam.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/accuracycoin_tracer.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_sprites.c
  - tests/runahead_model.h
findings:
  critical: 0
  warning: 2
  info: 1
  total: 3
status: issues_found
---

# Phase 19: Code Review Report

**Reviewed:** 2026-10-25
**Depth:** standard
**Files Reviewed:** 18
**Status:** issues_found

## Summary

Reviewed the diff since deb7349 for the PPU sprite-0 and OAMADDR changes, the libretro presentation-cache timeline tracking, and the runner JAM reporting. I traced the PPU evaluation state machine (`eval_n` stays below 64 after `oam_addr >> 2`, so primary OAM indexing is in bounds). I also checked the state-CRC and frame-number offsets against `src/state.c`. The state CRC covers the frame number, so the libretro identity check is sound. I found no crashes, security issues or incorrect behaviour. I did not read the large test-side diffs line by line (`tests/libretro/libretro_host.c`, `tests/runahead_model.h`, `tests/ppu/test_sprites.c`, `tests/accuracy/test_conformance.c`). Two robustness concerns and one note follow.

## Warnings

### WR-01: Sprite-0 hit relies on an implicit coupling to the video path

**File:** `src/ppu.c:427-431`
**Issue:** The sprite-0 hit is set only inside `compose_pixel`, which runs only when a video buffer is present. The new comment justifies this with "nesturbator_run_frame rejects NULL video" (`src/frame.c:20`). Any future headless or skip-render mode, or a second caller of the PPU tick, would silently stop setting the sprite-0 flag and change emulation. That would break the determinism rule, because the hit would then depend on output. The invariant lives only in a comment.
**Fix:** Evaluate the hit in a function that is separate from pixel output. Alternatively add a test that fails if `run_frame` ever accepts NULL video.

### WR-02: The libretro cache treats all later JAM frames as one identity

**File:** `libretro/libretro.c` (`retro_run` STOP_JAM branch, `retro_unserialize`)
**Issue:** `jam_state_crc` keeps a single value, overwritten on every JAM frame. If the machine's state changes per JAM tick (for example ticks, cycle counters or PPU position), then loading an earlier jammed state during run-ahead yields a CRC that is neither `cached_state_crc` nor the latest `jam_state_crc`. The adapter moves to a new timeline and shows black. This degrades the behaviour D-16 set out to fix, and only after the first JAM frame. The D-16 comment documents only the first-real-frame case. I did not confirm whether the jammed state is frame-invariant.
**Fix:** Verify with a test that run-ahead across several consecutive JAM frames still shows the cached picture. If the jammed state is not frame-invariant, record the crc only once per JAM entry and key the identity on `frame_number` plus the cached CRC.

## Info

### IN-01: A full state save for every converted frame

**File:** `libretro/libretro.c` (`retro_run`, `scratch_state_crc`)
**Issue:** A `nesturbator_save_state` plus CRC runs on every converted frame (and every JAM frame while the cache is current) only to track presentation identity. Performance is out of v1 scope, so this is a note. The cost does matter for the retro_run hot path with run-ahead.
**Fix:** If it becomes a concern, compute the CRC lazily, only in `retro_unserialize` or when a JAM is first seen.

---

_Reviewed: 2026-10-25_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
