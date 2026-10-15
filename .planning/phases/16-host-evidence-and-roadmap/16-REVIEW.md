---
phase: 16-host-evidence-and-roadmap
reviewed: 2026-10-15T01:50:09Z
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
  warning: 2
  info: 0
  total: 2
status: issues_found
---

# Phase 16: Code Review Report

**Reviewed:** 2026-10-15T01:50:09Z
**Depth:** standard
**Files Reviewed:** 16
**Status:** issues_found

## Summary

The three findings from the first review are addressed: setup failures now release the acquired session lock, archive paths containing semicolons are rejected, and duplicate localized support clauses fail the metadata guard. The current three build archives pass the checker. The recheck also found two remaining false-acceptance cases in archive inventory: newline-bearing member names are confused with listing separators, and duplicate expected member names are not rejected.

## Narrative Findings (AI reviewer)

## Warnings

### WR-04: Newline in an archive member name can impersonate multiple expected files

**File:** `tests/cmake/check_archives.cmake:114-149`
**Classification:** WARNING
**Issue:** The checker rejects semicolons, then treats every newline in `cmake -E tar tf` output as an entry separator. A ZIP member whose literal name is `LICENSE\nbin/nesturbator-run` is therefore seen as two allowlisted files. I independently created a three-package synthetic archive set with that single runner entry; the current checker accepted all three packages with exit code 0. The archive can consequently omit distinct required files while passing the exact-content gate.
**Fix:** Read ZIP central-directory names through an API that returns each member as a structured value, reject control characters in each complete name, and compare those names against the allowlist without line-based splitting.

### WR-05: Duplicate allowlisted archive entries pass the exact-content gate

**File:** `tests/cmake/check_archives.cmake:127-149`
**Classification:** WARNING
**Issue:** For each expected pattern, the checker records only whether any match exists; it then accepts every listed file that matches any pattern. It never rejects a repeated path. I independently created a runner ZIP containing two `LICENSE` entries plus its runner binary; the current checker accepted the package set with exit code 0. Duplicate ZIP member names can make extraction select ambiguous content while the checker reports the package as exact.
**Fix:** Track exact member names and reject duplicates, and require each expected pattern to match exactly one archive entry before accepting the package.

---

_Reviewed: 2026-10-15T01:50:09Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
