---
phase: 01-a-test-frame-in-retroarch
fixed_at: 2026-10-08T15:24:57Z
review_path: .planning/phases/01-a-test-frame-in-retroarch/01-REVIEW.md
iteration: 1
findings_in_scope: 2
fixed: 2
skipped: 0
status: all_fixed
---

# Phase 1: Code Review Fix Report

**Fixed at:** 2026-10-08T15:24:57Z
**Source review:** `.planning/phases/01-a-test-frame-in-retroarch/01-REVIEW.md`
**Iteration:** 1

**Summary:**
- Findings in scope: 2
- Fixed: 2
- Skipped: 0

## Fixed Issues

### WR-01: Calling the allocator member can expand a function-like `free` macro

**Files modified:** `src/instance.c`
**Commit:** `e4830fb`
**Applied fix:** Parenthesized the `free` callback member before invocation to suppress function-like macro expansion.
**Verification:** Re-read the changed function and built target `nesturbator` successfully in the main checkout.

### WR-02: The vector harness count parser can accept overflowing inputs

**Files modified:** `tests/cpu/test_vectors.c`, `tests/CMakeLists.txt`
**Commits:** `aa9b83d` (parser fix), `a892a6c` (overflow regression tests)
**Applied fix:** Check the next decimal digit against the maximum before multiplying and adding, preventing overflow from wrapping into an accepted count.
**Verification:** Re-read the parser and built target `cpu.vectors` successfully in the main checkout. Added and passed `cpu.vectors.usage.overflow.chunks` and `cpu.vectors.usage.overflow.per_chunk`, which verify oversized decimal limits exit with usage error 2.

## Verification Environment

The focused builds and full CI workflow ran in the main checkout. Configure and build passed. The final `cmake --workflow --preset ci` run passed 330 of 331 tests, including both overflow regressions; `retroarch.testframe` aborted because RetroArch did not start successfully in this environment. Its harness requires a logged-in macOS GUI session. No manual verification remains for WR-01 or WR-02.

---

_Fixed: 2026-10-08T15:24:57Z_
_Fixer: the agent (gsd-code-fixer)_
_Iteration: 1_
