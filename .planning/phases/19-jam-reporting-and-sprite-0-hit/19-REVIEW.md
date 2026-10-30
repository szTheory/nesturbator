---
phase: 19-jam-reporting-and-sprite-0-hit
reviewed: 2026-10-30T04:12:54Z
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
  warning: 0
  info: 1
  total: 1
status: issues_found
---

# Phase 19: Code Review Report

**Reviewed:** 2026-10-30T04:12:54Z

**Depth:** standard

**Files Reviewed:** 18

**Status:** issues_found

## Summary

Reviewed the current Phase 19 implementation scope across the six plan summaries: AccuracyCoin JAM reporting and collection, libretro presentation across run-ahead state loads, and PPU OAMADDR/sprite-0 evaluation. The evaluation-scope resolver returned `degraded` (`no-task-commit-rows`) and widened to the phase-directory range, which includes later phases. I therefore used the six summaries' `key-files` union as the Phase 19 scope and applied the workflow's planning/data exclusions; the 18 listed files are the remaining source and test/build files. The report has one Info quality finding and no confirmed behavior or security defects. No tests were run as part of this review.

Historical findings and their original decisions are retained below. The current finding uses a new ID so the original dispositions remain attached to the same issues.

## Narrative Findings (AI reviewer)

### IN-02: Sprite-0 comparator comment cites unrelated hardware sections

**Severity:** Info

**File:** `src/ppu.c:428-429`

**Issue:** The comment's hardware assertion that sprite-0 collision depends on pattern opacity rather than palette color is cited to HWP.03 (frame timing/NMI) and HWP.07 (palettes). The preparation index maps sprite evaluation and sprite-0 hit behavior to HWP.06. The current citations do not let a maintainer verify the comparator rule from the project's pinned hardware notes.

**Fix:** Cite `[HWP.06]` for the sprite-0 collision rule. Keep `[HWP.03]` only for the status-latch clearing timing if it is the supporting source for that sentence, and remove `[HWP.07]` from the collision assertion.

## Historical Review History

The original standard review was recorded on 2026-10-25 against 18 files. It found no crashes or security issues, and its dispositions were recorded in `19-REVIEW-DISPOSITION.md`.

- **WR-01 — Sprite-0 hit relies on an implicit coupling to the video path** (original Warning; fixed). The original concern was that the hit was set only inside `compose_pixel`, which ran only when a video buffer was present. The fix report added a state test proving `nesturbator_run_frame` rejects a null video buffer without changing machine state. Current `src/ppu.c` also calls `detect_sprite_zero_hit` from the PPU tick path independently of pixel composition (lines 421-444 and 558-561).
- **WR-02 — The libretro cache treats all later JAM frames as one identity** (original Warning; fixed). The original concern was that a later JAM state might have a different CRC from the cached frame and the recorded jam state. The fix report added a state test proving repeated JAM calls leave the serialized bytes unchanged; the single `jam_state_crc` identity is still consistent with that behavior.
- **IN-01 — A full state save for every converted frame** (original Info; open). The original note records the cost of saving to scratch to identify the cached presentation timeline. It is retained as historical review context; performance is out of scope for this review.

---

_Reviewed: 2026-10-30T04:12:54Z_

_Reviewer: the agent (gsd-code-reviewer)_

_Depth: standard_
