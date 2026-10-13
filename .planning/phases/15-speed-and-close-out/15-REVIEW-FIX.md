---
phase: 15-speed-and-close-out
fixed_at: 2026-10-13T00:00:00Z
review_path: .planning/phases/15-speed-and-close-out/15-REVIEW.md
iteration: 1
findings_in_scope: 4
fixed: 3
skipped: 1
status: partial
---

# Phase 15: Code Review Fix Report

**Fixed at:** 2026-10-13
**Source review:** .planning/phases/15-speed-and-close-out/15-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 4
- Fixed: 3
- Skipped: 1

Verification ran in the main checkout (`workflow.use_worktrees` is false): `cmake --workflow --preset ci` passed 633 of 633 tests, and `scripts/hygiene.sh` exited 0.

## Fixed Issues

### WR-01: Perf gate passes silently when the head measurement is missing and the base is also missing

**Files modified:** `tests/cmake/perf_compare.cmake`, `tests/cmake/perf_gate_selftest.cmake`, `tests/CMakeLists.txt`
**Commit:** 68f8f44
**Applied fix:** In gate mode a missing head is now tested before the missing-base branch, so it FAILs even when the base is missing. Added selftest `perf.gate_selftest.missing_head_and_base`, which expects exit 1. Status: fixed, requires human verification (logic change).

### WR-02: `file(STRINGS)` on the allow file mangles reasons that contain `;` or `[` `]`

**Files modified:** `tests/cmake/perf_compare.cmake`, `tests/cmake/perf_gate_selftest.cmake`, `tests/CMakeLists.txt`
**Commit:** 68f8f44
**Applied fix:** The allow file is read with `file(READ)`, the characters `[`, `]` and `;` are replaced with `_`, then it is split on newlines. Added selftests `allow_reason_semicolon`, `allow_reason_bracket` and `allow_bracket_then_malformed`. The last one checks that line numbers stay right after a line with `[`. The selftest writes these characters through `@SEMI@`, `@LB@` and `@RB@` placeholders because macro arguments would split on them.

### WR-03: An inherited SIG_IGN is honoured but not documented, and it can hang the stop tests

**Files modified:** `README.md`, `runner/stop.h`
**Commit:** 5845467
**Applied fix:** Both now say that a SIGINT or SIGTERM the runner inherited as ignored stays ignored. The README also says the `runner.signal.*` and `runner.stop.*` tests need those signals at their default. I did not change the test harness. POSIX shells cannot reset a signal that was ignored on entry (`trap - INT TERM` has no effect), and CMake cannot read the disposition, so there is no clean fail-fast or reset.

## Skipped Issues

### WR-04: One-minute job timeouts sit at about 2 to 3 times a single sample

**File:** `.github/workflows/release.yml:25`, `.github/workflows/ci.yml`
**Reason:** There is no measured run ID to back a new margin. Hosted CI cannot be run from here, and the review's fix (a 5-minute floor) would break the rule that every timeout carries a measured run and a "twice a measured run" comment, or it would require inventing evidence. The owner should collect further run samples, or decide on an explicit floor policy for network-bound jobs and update `hygiene.timeout_comments`.
**Original issue:** The `release-please`, `hygiene`, `title` and `ci-required` timeouts come from a single sample each, and the tail latency of network calls on hosted runners may exceed them.

---

_Fixed: 2026-10-13_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
