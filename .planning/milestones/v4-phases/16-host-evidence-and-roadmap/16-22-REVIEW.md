---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-17T00:34:57Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
  - tests/retroarch/interruption_trace.sh
  - tests/retroarch/frame_index.c
  - README.md
  - include/nesturbator.h
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/run_retroarch.cmake
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-17T00:34:57Z  
**Depth:** standard  
**Files Reviewed:** 8  
**Status:** issues_found

## Summary

Reviewed the required driver, harness, trace helper, C source, README, and public-header files, with the RetroArch CTest registration and wrapper as context. The ten-path source manifest in `build/phase16-native22/formal/candidate-source.tsv` matches the working blobs for candidate `3643d6ac04c8c09391a6f2ea68c7217ba4c5ba25` on `gsd/phase-16-host-evidence-and-roadmap`; current HEAD `46ece540c3b0b6edc50ae90ec4965c055a3fc2c3` adds the acceptance record. The supplied `16-22-NATIVE-REPAIR.md` evidence report has SHA256 `1e1b2f3305c3ee1391a59f22a4ecb08314282d235abecc5566185b1875ba9c44`.

The readiness and one-shot controls consistently check exact replies, request counts, attribution, bounded cleanup, and reaping. The owner-collision control exercises both shells that return from failed redirection and shells that exit, and checks the foreign object identity. One lock-release path still has a check-then-unlink race that can remove a replacement object.

## Narrative Findings (AI reviewer)

### CR-01: Lock cleanup can unlink a replacement object after its ownership check

**Severity:** BLOCKER  
**File:** `tests/retroarch/state_session.sh:48-57`  
**Issue:** `lock_owner_path_is_created` checks the current pathname's device and inode, then cleanup separately reads its token with `cat` and unlinks it with `rm`. A process able to replace `$LOCK_DIR`, `$LOCK_OWNER_ALIAS`, or `$LOCK_OWNER_FILE` after those checks but before `rm` can cause cleanup to delete the replacement (including the last link to a foreign regular file). The replacement test at `tests/retroarch/test_state_session.sh:1465-1472` replaces the owner pathname before signal cleanup starts, so it does not exercise replacement in the check-to-unlink interval. This breaks the promised preservation of foreign lock objects whenever the lock directory can be mutated concurrently.

**Fix:** Avoid conditional deletion of a shared pathname based on a prior `lstat`/content check. Use a persistent advisory-lock file that is not unlinked during release, or require and validate a private owner-controlled lock directory before creating entries there. Add a deterministic control that attempts replacement between the ownership check and cleanup and verifies the foreign entry survives.

---

_Reviewed: 2026-10-17T00:34:57Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
