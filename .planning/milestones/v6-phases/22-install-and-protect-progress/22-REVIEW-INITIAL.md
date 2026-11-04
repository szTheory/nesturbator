---
phase: 22-install-and-protect-progress
reviewed: 2026-10-31T01:50:22Z
depth: standard
files_reviewed: 11
files_reviewed_list:
  - README.md
  - docs/api.md
  - docs/progress.md
  - docs/releases.md
  - docs/runner.md
  - docs/testing.md
  - tests/CMakeLists.txt
  - tests/cmake/doc_links.cmake
  - tests/cmake/runner_exit_table.cmake
  - tests/cmake/runner_options.cmake
  - tests/cmake/status_table.cmake
findings:
  critical: 0
  warning: 3
  info: 0
  total: 3
status: issues_found
---

# Phase 22: Code Review Report

**Reviewed:** 2026-10-31T01:50:22Z
**Depth:** standard
**Files Reviewed:** 11
**Status:** issues_found

## Summary

Reviewed all eleven scoped documentation and CMake files, including the relocated API, runner, release and testing contracts, progress guidance, link checker and canonical-contract tests. The current test registrations use the intended guide paths, but the status checker does not enforce that path itself. The player install command and release attestation description also need clarifying to match configured RetroArch directories and the release workflow’s actual attestation subjects.

## Warnings

### WR-01: Apple Silicon install command assumes RetroArch’s default directories

**File:** `README.md:40-60`
**Issue:** The README first directs users to RetroArch’s configured Cores and Core Info directories, which can vary by installation, then says the Apple Silicon command installs into RetroArch’s directories. That command always extracts into `~/Library/Application Support/RetroArch`, so users with custom directory settings will install files where the active frontend does not look for them.
**Fix:** State that the marked command uses RetroArch’s default macOS data directory and instruct users with custom directories to change the `-C` destination to their configured location; keep the tested extraction operands and release marker intact.

### WR-02: Release guide names the wrong attestation subject

**File:** `docs/releases.md:166-171`
**Issue:** The guide says the workflow creates an attestation “for that checksum file.” In the workflow, `SHA256SUMS` is passed as `subject-checksums`, then each ZIP is individually checked with `gh attestation verify` (`.github/workflows/release.yml:109-117`). The current wording therefore misidentifies what the attestation proves, despite the later archive-verification sentence.
**Fix:** Say that `SHA256SUMS` supplies the archive subjects for the build-provenance attestations, and that the workflow verifies each listed archive against its attestation.

### WR-03: Status-table checker does not enforce the canonical API guide path

**File:** `tests/cmake/status_table.cmake:133-138`
**Issue:** The checker requires an existing `DOC` file but accepts any path. The current `tests/CMakeLists.txt:1626-1629` registration correctly passes `docs/api.md`, yet a later registration change could point it at an obsolete README copy and still satisfy the production check if that copy retained the matching table and warning. The runner checkers explicitly reject noncanonical document paths, but this checker has no equivalent guard or wrong-path control.
**Fix:** In production mode, normalize `SOURCE_DIR` and `DOC` and require `DOC` to equal `${SOURCE_DIR}/docs/api.md`; keep scratch self-test fixtures addressable under their fixture roots. Add a negative control that passes a stale noncanonical document and checks for the path-specific rejection.

---

_Reviewed: 2026-10-31T01:50:22Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
