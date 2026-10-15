---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-15T01:25:19Z
depth: standard
files_reviewed: 16
files_reviewed_list:
  - .github/workflows/ci.yml
  - CMakePresets.json
  - README.md
  - include/nesturbator.h
  - libretro/nesturbator_libretro.info
  - tests/CMakeLists.txt
  - tests/cmake/check_archives.cmake
  - tests/cmake/fixture_identity.cmake
  - tests/cmake/info_claims.cmake
  - tests/libretro/libretro_host.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/frame_index.c
  - tests/retroarch/progression.h
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
findings:
  critical: 0
  warning: 3
  info: 0
  total: 3
status: issues_found
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-15T01:25:19Z  
**Depth:** standard  
**Files Reviewed:** 16  
**Status:** issues_found

## Summary

The review traced the new RetroArch progression/session checks, fixture identity boundary, metadata guards, and package checks through their callers. Three robustness defects remain in the session lock cleanup and the exactness of archive/documentation validation; each can let a failed setup block later runs or allow the acceptance guards to miss invalid input.

## Narrative Findings (AI reviewer)

## Warnings

### WR-01: Setup failure can strand the RetroArch port lock

**File:** `tests/retroarch/state_session.sh:29-32,146`
**Classification:** WARNING
**Issue:** The script creates the exclusive lock directory at line 29, then creates the attempt tree and journal at lines 30-32. `set -e` exits on a failed `mkdir` or journal redirection, but the EXIT and signal traps that remove the lock are not installed until line 146. A full disk, permission change, or other setup I/O failure therefore leaves `nesturbator-retroarch-55355.lock` behind and causes subsequent attempts to fail as `ENDPOINT_BUSY` even though no host owns the port.
**Fix:** Install a minimal lock-release EXIT trap immediately after successfully creating `LOCK_DIR`, before any later fallible setup command. Replace it with the full cleanup traps once their state and functions are initialized.

### WR-02: Semicolons in archive paths bypass the exact file allowlist

**File:** `tests/cmake/check_archives.cmake:221-249`
**Classification:** WARNING
**Issue:** The archive listing is converted to a CMake list by replacing newlines with semicolons. A real archive entry named `LICENSE;THIRD-PARTY-NOTICES.md` is then treated as two entries; both satisfy the anchored allowlist patterns, so the checker can accept an archive missing the two required files while claiming every archive has exactly its expected contents. The subsequent loops validate the split list elements rather than the original listing entries.
**Fix:** Reject semicolons in the raw archive listing before converting it to a CMake list, or parse entries with an approach that preserves literal semicolons and compare each original entry exactly against the allowlist.

### WR-03: Documentation guard ignores later contradictory support claims

**File:** `tests/cmake/info_claims.cmake:152-165,221-249`
**Classification:** WARNING
**Issue:** `nesturbator_info__support_text` finds only the first `Supported NTSC ` occurrence, then validates only through that sentence's first period. The caller passes each complete README/header and metadata description, so a later support clause in the same text can claim a different MMC3 mapper or submapper and still pass as long as the first clause is correct. This leaves the guard unable to enforce the documented support boundary across the whole checked text.
**Fix:** Scan and validate every occurrence of the support-clause prefix, or reject duplicate support clauses after validating the one canonical clause.

---

_Reviewed: 2026-10-15T01:25:19Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
