---
phase: 21-interrupt-latency-and-hijack
reviewed: 2026-10-29T19:23:48Z
depth: standard
files_reviewed: 9
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/phase21_source_identity.cmake
  - tests/cmake/phase21_source_identity_controls.cmake
findings:
  critical: 0
  warning: 1
  info: 0
  total: 1
status: issues_found
---

# Phase 21: Incremental Code Review Report

**Reviewed:** 2026-10-29T19:23:48Z
**Depth:** standard
**Files Reviewed:** 9
**Status:** issues_found

## Summary

Reviewed the nine requested tracked files changed since `5cf4c9c`. The IRQ refactor preserves the operation-specific poll points in the changed branches, and the reported performance repair retains its original CI and timeout gates. The source-identity manifest still omits the actual working-tree mode of regular files, so a chmod-only change can pass verification. Phase 21 CR-01 remains rejected and CR-02 remains fixed as documented in [21-REVIEW-DISPOSITION.md](21-REVIEW-DISPOSITION.md). Phase 20 collector CR-01, WR-03, and WR-04 remain deferred; the run-specific mitigations documented in [21-EVIDENCE.md](21-EVIDENCE.md) do not repair those recurring collector defects.

## Narrative Findings (AI reviewer)

### WR-01: Worktree executable-mode changes are absent from source identity (WARNING)

**File:** `tests/cmake/phase21_source_identity.cmake:83-93, 134-142`; `tests/cmake/phase21_source_identity_controls.cmake:99-102`
**Issue:** For regular files, the helper records committed and index modes, then hashes only the worktree bytes. A chmod-only change to a tracked executable leaves every manifest field unchanged, so `MODE=verify` accepts a checkout whose executable behavior differs. The semantic control changes the index mode with `git update-index --chmod=+x`, but does not change the file's actual permissions.
**Fix:** Record a normalized worktree mode for regular files (and the symlink mode for links), compare it in `verify`, and add a POSIX control that chmods a file while leaving its index entry untouched and requires verification to reject it. Keep the committed, index, and worktree fields distinct.

---

_Reviewed: 2026-10-29T19:23:48Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
