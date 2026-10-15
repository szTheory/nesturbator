---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-15T02:05:07Z
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
  warning: 1
  info: 0
  total: 1
status: issues_found
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-15T02:05:07Z
**Depth:** standard
**Files Reviewed:** 16
**Status:** issues_found

## Summary

WR-01 through WR-05 are addressed: setup-failure lock cleanup, semicolon rejection, duplicate documentation-claim rejection, control-byte rejection, and duplicate/member-pattern rejection are present with corresponding controls. The focused archive self-test and CTest control pass, and all three current build archives pass. One remaining archive-inventory gap lets a ZIP's Unicode Path Extra Field make CMake list an allowed name that differs from the raw central-directory name.

## Narrative Findings (AI reviewer)

## Warnings

### WR-06: Unicode path metadata bypasses the raw archive-name inventory

**File:** `tests/cmake/check_archives.cmake:181-200,232-340`
**Classification:** WARNING
**Issue:** `nesturbator_archive_inventory_check` reads raw central-directory names and rejects control bytes and duplicate raw names, but it does not require those names to match the entries returned by `cmake -E tar tf`; its `member_hexes` result is not used by the caller. Libarchive honors the Info-ZIP Unicode Path Extra Field (`0x7075`) when listing names. I reproduced an accepted runner ZIP whose raw names were `odd-license` and `odd-runner`, with Unicode Path Extra Fields naming them `LICENSE` and `bin/nesturbator-run`. The checker listed the aliases as the two expected files and accepted the synthetic three-package set with exit code 0, even though neither expected runner path exists as a raw central-directory name.
**Fix:** Compare each raw central-directory name directly against the component's allowed file and directory names. Reject Unicode Path Extra Fields that override names, or verify that every decoded/listed name exactly matches its raw name before applying the allowlist.

---

_Reviewed: 2026-10-15T02:05:07Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
