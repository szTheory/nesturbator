---
phase: 19-jam-reporting-and-sprite-0-hit
fixed_at: 2026-10-25T00:00:00Z
review_path: .planning/phases/19-jam-reporting-and-sprite-0-hit/19-REVIEW.md
iteration: 1
findings_in_scope: 2
fixed: 2
skipped: 0
status: all_fixed
---

# Phase 19: Code Review Fix Report

**Fixed at:** 2026-10-25
**Source review:** .planning/phases/19-jam-reporting-and-sprite-0-hit/19-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 2
- Fixed: 2 (both by regression test; no source change needed)
- Skipped: 0

**Verification:** `cmake --workflow --preset ci` ran in the main checkout (workflow.use_worktrees is false) and passed before the commit.

## Fixed Issues

### WR-01: Sprite-0 hit relies on an implicit coupling to the video path

**Files modified:** `tests/core/test_state.c`
**Commit:** a37e0a7
**Applied fix:** Added a test (in `jam_both_ways`) that a non-jammed instance given `video == NULL` gets `NESTURBATOR_ERR_ARGUMENT` from `nesturbator_run_frame` and its saved state is byte-identical afterwards. The invariant in the `compose_pixel` comment is now enforced by a test instead of only by the comment. The PPU was not restructured.

### WR-02: The libretro cache treats all later JAM frames as one identity

**Files modified:** `tests/core/test_state.c`
**Commit:** a37e0a7
**Applied fix:** Determined by test that the jammed state is frame-invariant. `nesturbator_run_frame` returns `STOP_JAM` before touching state once `cpu.jammed` is set, so further JAM frames leave the saved bytes unchanged (including frame number and CRC). The single `jam_state_crc` in `libretro/libretro.c` is therefore correct, and no code change was made. The new check runs three more JAM frames and asserts the saved state is byte-equal to the first jammed save. Status: fixed by test, not by code change (no logic edit; the reviewer's concern was unconfirmed and is now refuted).

---

_Fixed: 2026-10-25_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
