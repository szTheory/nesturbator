---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-23T18:38:33Z
depth: standard
files_reviewed: 6
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/interruption_trace.sh
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
status: clean
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-23T18:38:33Z

**Depth:** standard

**Files Reviewed:** 6

**Reviewed HEAD:** `baefa5e687f7f0ee5320dededdcc4ed3ad0432e5`, plus the current worktree delta in `tests/retroarch/CMakeLists.txt`, `README.md`, and `include/nesturbator.h`

**Diff base:** `dfc4f66e9b92aa4395096916b5aefaaff20176c3`

**Status:** clean

## Summary

The initial review found one test-coverage gap, recorded as WR-07 below. The follow-up inspected the exact worktree fix: CTest now registers the source-root preflight as a serial POSIX test, and the README and public header describe its CI and ASan lane coverage. WR-07 is resolved; no current findings remain. The Plan 16-56 exact-source review and disposition remain separate historical evidence.

## Warnings (Initial Review Snapshot; Resolved in Follow-up)

### WR-07: Source-root preflight was not run by the registered CI tests

**Severity:** WARNING (initial review)

**Initial source:** `tests/retroarch/CMakeLists.txt:71-88`; `tests/retroarch/test_state_session.sh:3770-3807`

**Initial issue:** At the initial review snapshot, CMake registered `retroarch.session.controls` and `retroarch.session.readiness`, but neither selected `STATE_SESSION_CONTROL=source-root-preflight`. The controls test used the default `CONTROL=all`, which did not call `source_root_preflight`. Thus the copied-input refusal and the three-working-directory matrix with twelve CWD-specific finalizer cases were not part of the registered CI tests.

**Initial fix suggestion:** Register a serial `retroarch.session.source_root` CTest entry using `STATE_SESSION_CONTROL=source-root-preflight` and a bounded timeout, or call the preflight from the serial `all` control.

## Narrative Findings (AI reviewer)

No current findings. WR-07 is closed by the follow-up below.

## Follow-up Review

**Reviewed worktree:** HEAD `baefa5e687f7f0ee5320dededdcc4ed3ad0432e5` plus the current worktree delta.

WR-07 is fixed. `tests/retroarch/CMakeLists.txt:71-79` registers `retroarch.session.source_root` with the same harness arguments, sets `STATE_SESSION_CONTROL=source-root-preflight`, and applies `TIMEOUT 120` and `RUN_SERIAL TRUE`. The existing selector dispatches to the preflight at `tests/retroarch/test_state_session.sh:3770-3772`. Paired documentation now identifies the serial test and its POSIX `ci`/`asan` lane coverage at `README.md:220-225` and `include/nesturbator.h:546-550`.

The focused CTest result provided with the fix was 1/1 passed in 6.46 seconds. The full `ci` workflow was still running when this follow-up was written; this report makes no full-workflow pass claim.

---

_Reviewed: 2026-10-23T18:38:33Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
