---
phase: 11-tune-up-and-state-readiness
fixed_at: 2026-10-11T00:00:00Z
review_path: .planning/phases/11-tune-up-and-state-readiness/11-REVIEW.md
iteration: 1
findings_in_scope: 4
fixed: 4
skipped: 0
status: all_fixed
---

# Phase 11: Code Review Fix Report

**Fixed at:** 2026-10-11
**Source review:** .planning/phases/11-tune-up-and-state-readiness/11-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 4
- Fixed: 4
- Skipped: 0

Verification ran in the main checkout (workflow.use_worktrees is false), not a worktree.

## Fixed Issues

### WR-01: save.h promises a symlink at the .tmp name is "never removed", but the helper removes it

**Files modified:** `runner/save.c`, `runner/save.h`, `tests/runner/test_save_replace.c`, `README.md`
**Commit:** e5544fa
**Applied fix:** The stale-file probe uses `lstat` (mapped to `_stat` on Windows), so a symlink is judged as a link. The header, test header and README now say a regular file or symlink at `.tmp` is removed (a link is unlinked, never its target) and a directory is refused. Case (h) asserts return 0, the new bytes, an intact victim and no `.tmp`. The concurrent-runner race is documented as unsupported rather than changed, because dropping the pre-remove would make a stale `.tmp` wedge saves.
Status: fixed (logic change; a human may want to confirm the lstat semantics).

### WR-02: A dangling symlink at the .tmp name fails every save permanently

**Files modified:** `runner/save.c`, `tests/runner/test_save_replace.c`, `README.md`
**Commit:** a331a6d
**Applied fix:** `lstat` from WR-01 clears a dangling link. New case (i) shows the save succeeds and nothing is created at the link's target. The `fopen` failure message now names the `.tmp` path.

### WR-03: "sync" is not a durable flush on macOS, and the CI RetroArch job runs only on macOS

**Files modified:** `runner/save.c`, `runner/save.h`, `README.md`
**Commit:** 98dd4af
**Applied fix:** Under `#elif defined(F_FULLFSYNC)` the sync calls `fcntl(F_FULLFSYNC)` and falls back to `fsync` if that fails. `_DARWIN_C_SOURCE` is defined on Apple so the constant is visible under strict POSIX. The header and README mention it. The happy-path test exercises the call on macOS.

### WR-04: The chmod sub-case and the symlink sub-case in test_save_replace.c can pass without testing anything

**Files modified:** `tests/runner/test_save_replace.c`
**Commit:** f26cbca
**Applied fix:** The symlink return value is already asserted (WR-01, and `CHECK(symlink(...) == 0)` was present). In the chmod case, when the user is not blocked (root), the test now asserts the privileged write succeeds with the new bytes and no `.tmp`, instead of only printing a note.

Follow-up commit 7d1d609 re-wraps the `save.h` comment for clang-format.

## Verification

- `cmake --workflow --preset ci`: 100% passed, 488 of 488 (before each commit).
- `cmake --workflow --preset asan`: 100% passed, 483 of 483.
- `cmake --workflow --preset hygiene`: 8 of 8 passed after commit cc957f3, which applies clang-format 18 (the CI image's version) to `runner/main.c`, `runner/save.c`, `tests/core/test_boundary.c` and `tests/runner/test_save_replace.c`. The violations predated these fixes. `ci` still passes 488 of 488 after that commit.

---

_Fixed: 2026-10-11_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
