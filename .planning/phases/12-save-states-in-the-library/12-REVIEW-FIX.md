---
phase: 12-save-states-in-the-library
fixed_at: 2026-10-12T00:00:00Z
review_path: .planning/phases/12-save-states-in-the-library/12-REVIEW.md
iteration: 1
findings_in_scope: 4
fixed: 3
skipped: 1
status: partial
---

# Phase 12: Code Review Fix Report

**Fixed at:** 2026-10-12
**Source review:** .planning/phases/12-save-states-in-the-library/12-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 4
- Fixed: 3
- Skipped: 1

Verification: `cmake --workflow --preset ci` ran in the main checkout (`workflow.use_worktrees` is false, so no worktree was made) after the CR-01/WR-01 changes and again after IN-01: 100% of 526 tests passed.

## Fixed Issues

### CR-01: A CRC-valid state with `ticks` near 2^64 hangs the next frame

**Files modified:** `src/state.c`, `include/nesturbator.h`, `README.md`, `tests/core/test_state_reject.c`
**Commit:** c6c1c69
**Applied fix:** VALIDATE now refuses a header `ticks` above `STATE_TICKS_MAX` (2^62) with `NESTURBATOR_ERR_STATE_INVALID`, in the same cross-field check as the `ppu_ticks` gap (D-04 order unchanged). `ppu_ticks <= ticks`, so it is bounded too. The other u64 stamps (`cpu_cycle`, MMC1 `last_write`, MMC3 `low_cycle`) are used only by equality or wrapping difference, never as a loop bound, so they need no cap. Header check 7 and the README describe the bound. `core.state_reject` case 8 gains three CRC-recomputed rows: `UINT64_MAX` and `2^62 + 1` are refused, and `2^62` loads and one frame runs to `NESTURBATOR_OK`. There is no separate field-bounds recorder entry, because the bound is a cross-field rule on the header, not a state field.

### WR-01: Frame-range checks ran after ROM load

**Files modified:** `runner/main.c`, `tests/CMakeLists.txt`
**Commit:** 3e93170
**Applied fix:** New `check_frame_limits` runs the K-independent "beyond `--frames`" checks (`--hash-frame`, `--dump-frame`, `--state-at-frame`, `--hash-state`) right after the movie frame count is settled and before the instance, ROM or battery is touched. `check_ranges` keeps only the K-dependent comparisons after the state load. Two new tests, `runner.usage.beyond.badrom` and `runner.usage.dump.badrom`, show exit 2 even when the ROM path is missing.

### IN-01: VALIDATE comment contradicted the primitives

**Files modified:** `src/state.c`
**Commit:** 54b318e
**Applied fix:** The comment now says VALIDATE "changes nothing in the instance" and notes the primitives pass the field value along but ignore it. The primitives are unchanged. The commit also adds the optional WR-02 note below.

## Skipped Issues

### WR-02: `cpu.halted_in_read` validated as exactly 0

**File:** `src/state.c:104-105`
**Reason:** No change to the bound, as the orchestrator triaged. Nothing in `src` writes `halted_in_read`, and `src/internal.h` documents it as unused, so 0 is correct today. A comment at the field in `src/state.c` now says the bound must widen when stalled reads are modelled (committed with IN-01, 54b318e).
**Original issue:** The field is validated as 0 although `internal.h` describes it as used.

---

_Fixed: 2026-10-12_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
